// BEN DROWNED Builder: a small Windows app that builds the hack from your own Majora's Mask (USA) ROM.
//
// Drag the ROM onto the window, pick your options and press Build. Behind the scenes it does what the PowerShell
// setup does: it makes sure WSL and Ubuntu exist, downloads or updates the project inside Ubuntu, installs the build
// tools and runs `./ben setup`. The finished ROM (and optional .bps patch) lands in C:\Users\<you>\BEN-DROWNED.
//
// Build options: every entry in `Options` with an OptKey is written as `./ben config OPT_<KEY> 0|1` before a build,
// which `ben` turns into -DBEN_<KEY>=0|1 (see include/ben/ben.h). To add a ROM option later, add a #ifndef-guarded
// BEN_<KEY> switch to ben.h and one line to `Options` below.
//
// Built by .github/workflows/builder.yml (csc on Windows); it also compiles with Mono's mcs for a quick check:
//   cd windows/builder && mcs -target:winexe -r:System.Windows.Forms.dll -r:System.Drawing.dll -win32icon:ben.ico \
//     -resource:ben.ico,BenDrowned.ben.ico -resource:ben.png,BenDrowned.ben.png \
//     -resource:laugh.wav,BenDrowned.laugh.wav -out:BEN-Builder.exe *.cs

using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Drawing.Text;
using System.IO;
using System.Runtime.InteropServices;
using System.Security.Cryptography;
using System.Text;
using System.Threading;
using System.Windows.Forms;

namespace BenDrowned
{
    // One checkbox in the Options box.
    sealed class BuildOption
    {
        public string Key;     // settings key
        public string OptKey;  // when set, a ROM option passed to the build as OPT_<OptKey>; null for app-only options
        public string Label;
        public string Hint;
        public bool Default;
        public CheckBox Box;

        public BuildOption(string key, string optKey, string label, string hint, bool def)
        {
            Key = key;
            OptKey = optKey;
            Label = label;
            Hint = hint;
            Default = def;
        }
    }

    // The top of the window: BEN's face torn into strips and the title split red and cyan, like a cartridge that isn't
    // quite right. Every so often the tear shifts for a moment, and it goes wild when Build is pressed.
    sealed class Header : Control
    {
        readonly Image face;
        readonly Random random = new Random();
        readonly System.Windows.Forms.Timer timer = new System.Windows.Forms.Timer();
        const int RestSeed = 12345; // the tear the header shows when nothing is happening
        int glitchFrames;
        int glitchSeed;

        public Header(Image face)
        {
            this.face = face;
            SetStyle(ControlStyles.AllPaintingInWmPaint | ControlStyles.OptimizedDoubleBuffer | ControlStyles.UserPaint |
                         ControlStyles.ResizeRedraw,
                     true);
            timer.Interval = 70;
            timer.Tick += delegate { Tick(); };
            timer.Start();
        }

        public void Shock()
        {
            glitchFrames = 18;
            Invalidate();
        }

        void Tick()
        {
            if (glitchFrames > 0) {
                glitchFrames--;
            } else if (random.Next(120) == 0) {
                glitchFrames = 2 + random.Next(4);
            } else {
                return;
            }
            glitchSeed = random.Next();
            Invalidate();
        }

        protected override void Dispose(bool disposing)
        {
            if (disposing) {
                timer.Dispose();
            }
            base.Dispose(disposing);
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            var g = e.Graphics;
            g.SmoothingMode = SmoothingMode.AntiAlias;
            g.TextRenderingHint = TextRenderingHint.AntiAliasGridFit;
            bool glitching = glitchFrames > 0;
            var r = new Random(glitching ? glitchSeed : RestSeed);

            g.Clear(Color.Black);

            var faceArea = new Rectangle(16, 8, Height - 16, Height - 16);
            if (face != null) {
                DrawFace(g, faceArea, r);
            }

            int left = faceArea.Right + 18;
            using (var titleFont = new Font(BuilderForm.SerifFont, 30f, FontStyle.Bold))
            using (var taglineFont = new Font(BuilderForm.SerifFont, 11.5f, FontStyle.Italic))
            using (var bodyFont = new Font("Segoe UI", 9.75f)) {
                const string title = "BEN DROWNED";
                var at = new PointF(left, 22);

                // A red glow behind the title.
                using (var glow = new SolidBrush(Color.FromArgb(34, BuilderForm.ColAccentHot))) {
                    for (int dx = -3; dx <= 3; dx += 2) {
                        for (int dy = -3; dy <= 3; dy += 2) {
                            g.DrawString(title, titleFont, glow, at.X + dx, at.Y + dy);
                        }
                    }
                }
                // Split the title into red and cyan, like a bad signal.
                float split = 2 + r.Next(5);
                using (var cyan = new SolidBrush(Color.FromArgb(150, 40, 220, 220)))
                using (var red = new SolidBrush(Color.FromArgb(170, 255, 30, 40))) {
                    g.DrawString(title, titleFont, cyan, at.X - split, at.Y + r.Next(-2, 3));
                    g.DrawString(title, titleFont, red, at.X + split, at.Y + r.Next(-2, 3));
                }
                using (var ink = new SolidBrush(BuilderForm.ColText)) {
                    g.DrawString(title, titleFont, ink, at);
                }

                string tagline = glitching && r.Next(3) == 0 ? "You shouldn't have done that..."
                                                             : "You've met with a terrible fate, haven't you?";
                using (var gold = new SolidBrush(BuilderForm.ColGold)) {
                    g.DrawString(tagline, taglineFont, gold, left + 3, 76);
                }
                using (var dim = new SolidBrush(BuilderForm.ColDim)) {
                    g.DrawString("Builds the hack from your own Majora's Mask (USA) ROM.", bodyFont, dim, left + 3, 104);
                }
            }

            // A thin purple line fading out to the right.
            var line = new Rectangle(0, Height - 1, Width, 1);
            using (var fade = new LinearGradientBrush(new Rectangle(0, 0, Math.Max(1, Width), 1), BuilderForm.ColPurple,
                                                      Color.FromArgb(0, BuilderForm.ColPurple),
                                                      LinearGradientMode.Horizontal)) {
                g.FillRectangle(fade, line);
            }
        }

        void DrawFace(Graphics g, Rectangle area, Random r)
        {
            // Tear the picture into strips and shove some sideways.
            const int strips = 10;
            for (int i = 0; i < strips; i++) {
                int sy = face.Height * i / strips;
                int sh = face.Height * (i + 1) / strips - sy;
                int dy = area.Y + area.Height * i / strips;
                int dh = area.Y + area.Height * (i + 1) / strips - dy;
                int shove = r.Next(3) == 0 ? r.Next(-14, 15) : 0;
                g.DrawImage(face, new Rectangle(area.X + shove, dy, area.Width, dh), new Rectangle(0, sy, face.Width, sh),
                            GraphicsUnit.Pixel);
            }
            using (var wash = new SolidBrush(Color.FromArgb(45, BuilderForm.ColAccent))) {
                g.FillRectangle(wash, area);
            }

            // Let the edges of the picture sink into the dark.
            var outer = area;
            outer.Inflate(area.Width / 4, area.Height / 4);
            using (var path = new GraphicsPath()) {
                path.AddEllipse(outer);
                using (var fade = new PathGradientBrush(path)) {
                    fade.CenterColor = Color.FromArgb(0, 0, 0, 0);
                    fade.SurroundColors = new[] { Color.Black };
                    fade.FocusScales = new PointF(0.55f, 0.55f);
                    g.FillRectangle(fade, area);
                }
            }
        }
    }

    // A thin bar that sweeps blood red while work is running (the stock progress bar is Windows blue).
    sealed class PulseBar : Control
    {
        readonly System.Windows.Forms.Timer timer = new System.Windows.Forms.Timer();
        int position;
        bool running;

        public PulseBar()
        {
            SetStyle(ControlStyles.AllPaintingInWmPaint | ControlStyles.OptimizedDoubleBuffer | ControlStyles.UserPaint,
                     true);
            timer.Interval = 30;
            timer.Tick += delegate {
                position += 7;
                Invalidate();
            };
        }

        public bool Running
        {
            get { return running; }
            set {
                running = value;
                timer.Enabled = value;
                Invalidate();
            }
        }

        protected override void Dispose(bool disposing)
        {
            if (disposing) {
                timer.Dispose();
            }
            base.Dispose(disposing);
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            e.Graphics.Clear(BuilderForm.ColPanel);
            if (!running || Width < 1) {
                return;
            }
            int length = Math.Max(80, Width / 3);
            int x = position % (Width + length) - length;
            var sweep = new Rectangle(x, 0, length, Height);
            using (var brush = new LinearGradientBrush(sweep, Color.Black, Color.Black, LinearGradientMode.Horizontal)) {
                brush.InterpolationColors = new ColorBlend {
                    Colors = new[] {
                        Color.FromArgb(0, BuilderForm.ColAccent), BuilderForm.ColAccentHot,
                        Color.FromArgb(0, BuilderForm.ColAccent),
                    },
                    Positions = new[] { 0f, 0.5f, 1f },
                };
                e.Graphics.FillRectangle(brush, sweep);
            }
        }
    }

    sealed class BuilderForm : Form
    {
        const string RepoUrl = "https://github.com/StormEf4/BEN-DROWNED.git";
        const string ProjectFolder = "BEN-DROWNED";
        const string RetailMd5 = "2a0a8acb61538235bc1094d297fb6556"; // Majora's Mask (USA), .z64
        const long RomSize = 32L * 1024 * 1024;

        // Majora's Mask after dark: near-black with a purple cast, bone-white text, blood red and the mask's gold.
        internal static readonly Color ColBack = Color.FromArgb(10, 7, 13);
        internal static readonly Color ColPanel = Color.FromArgb(22, 15, 27);
        internal static readonly Color ColPanelHot = Color.FromArgb(36, 22, 42);
        internal static readonly Color ColText = Color.FromArgb(228, 216, 198);
        internal static readonly Color ColDim = Color.FromArgb(146, 128, 152);
        internal static readonly Color ColAccent = Color.FromArgb(150, 18, 26);
        internal static readonly Color ColAccentHot = Color.FromArgb(200, 34, 42);
        internal static readonly Color ColPurple = Color.FromArgb(96, 46, 122);
        internal static readonly Color ColGold = Color.FromArgb(206, 160, 70);
        internal static readonly Color ColGood = Color.FromArgb(130, 196, 122);
        internal static readonly Color ColBad = Color.FromArgb(232, 100, 90);

        internal const string SerifFont = "Palatino Linotype";

        readonly List<BuildOption> Options = new List<BuildOption>
        {
            new BuildOption("skipIntro", "SKIP_INTRO", "Skip the intro",
                            "New files start in South Clock Town as human Link. Handy for testing.", true),
            new BuildOption("makePatch", null, "Also make a shareable patch (.bps)",
                            "For sending to friends. They apply it to their own ROM.", false),
            new BuildOption("openFolder", null, "Open the folder when it's done", null, true),
            new BuildOption("laugh", null, "Let the Happy Mask Salesman laugh when you press Build", null, true),
        };

        readonly Dictionary<string, string> settings = new Dictionary<string, string>();
        readonly string settingsPath = Path.Combine(
            Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData), "BEN-DROWNED", "builder.ini");
        readonly string outputFolder = Path.Combine(
            Environment.GetFolderPath(Environment.SpecialFolder.UserProfile), "BEN-DROWNED");

        readonly ConcurrentQueue<string> pendingLog = new ConcurrentQueue<string>();
        readonly System.Windows.Forms.Timer logTimer = new System.Windows.Forms.Timer();

        Header header;
        Label dropLabel;
        Label romStatus;
        Panel dropPanel;
        bool dropHot;
        Button buildButton;
        Button playButton;
        Button folderButton;
        Label statusLabel;
        PulseBar progress;
        TextBox logBox;

        string romPath;
        volatile bool busy;

        public BuilderForm()
        {
            Text = "BEN DROWNED Builder";
            BackColor = ColBack;
            ForeColor = ColText;
            Font = new Font("Segoe UI", 9.75f);
            ClientSize = new Size(620, 800);
            MinimumSize = new Size(560, 760);
            StartPosition = FormStartPosition.CenterScreen;
            DoubleBuffered = true;
            SetStyle(ControlStyles.ResizeRedraw, true);
            AllowDrop = true;
            DragEnter += OnDragEnter;
            DragDrop += OnDragDrop;
            DragLeave += delegate { SetDropHot(false); };

            try {
                var iconStream = Resource("ben.ico");
                if (iconStream != null) {
                    Icon = new Icon(iconStream);
                }
            } catch (Exception) {
                // Without the icon the app still works.
            }

            LoadSettings();
            BuildLayout();

            logTimer.Interval = 200;
            logTimer.Tick += delegate { FlushLog(); };
            logTimer.Start();

            string savedRom;
            if (settings.TryGetValue("rom", out savedRom) && File.Exists(savedRom)) {
                SetRom(savedRom);
            }
            Log("Ready. Drag your ROM in, then press Build.");


        }

        static Stream Resource(string name)
        {
            return typeof(BuilderForm).Assembly.GetManifestResourceStream("BenDrowned." + name);
        }

        // Dark title bar on Windows 10 and 11 (and a near-black one on 11), so the window frame doesn't glow white.
        [DllImport("dwmapi.dll")]
        static extern int DwmSetWindowAttribute(IntPtr window, int attribute, ref int value, int size);

        // Dark scroll bars for the log.
        [DllImport("uxtheme.dll", CharSet = CharSet.Unicode)]
        static extern int SetWindowTheme(IntPtr window, string appName, string idList);

        protected override void OnHandleCreated(EventArgs e)
        {
            base.OnHandleCreated(e);
            try {
                int on = 1;
                if (DwmSetWindowAttribute(Handle, 20, ref on, 4) != 0) {
                    DwmSetWindowAttribute(Handle, 19, ref on, 4); // older Windows 10 builds
                }
                int caption = ColBack.R | (ColBack.G << 8) | (ColBack.B << 16);
                DwmSetWindowAttribute(Handle, 35, ref caption, 4);
                SetWindowTheme(logBox.Handle, "DarkMode_Explorer", null);
            } catch (Exception) {
                // Not Windows 10 or newer: keep the normal title bar.
            }
        }

        // A faint purple glow in the middle that sinks into black at the edges.
        protected override void OnPaintBackground(PaintEventArgs e)
        {
            e.Graphics.Clear(ColBack);
            var area = ClientRectangle;
            area.Inflate(area.Width / 3, area.Height / 4);
            using (var path = new GraphicsPath()) {
                path.AddEllipse(area);
                using (var brush = new PathGradientBrush(path)) {
                    brush.CenterColor = Color.FromArgb(40, ColPurple);
                    brush.SurroundColors = new[] { Color.FromArgb(230, 0, 0, 0) };
                    e.Graphics.FillRectangle(brush, ClientRectangle);
                }
            }
        }

        // ---------------------------------------------------------------------------------------------------------
        // Layout

        void BuildLayout()
        {
            Image face = null;
            try {
                var faceStream = Resource("ben.png");
                if (faceStream != null) {
                    face = Image.FromStream(faceStream);
                }
            } catch (Exception) {
                // The header just goes without the face.
            }
            header = new Header(face) {
                Location = new Point(0, 0),
                Size = new Size(ClientSize.Width, 156),
                Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right,
            };

            dropPanel = new Panel {
                Location = new Point(20, 168),
                Size = new Size(580, 100),
                BackColor = ColPanel,
                Cursor = Cursors.Hand,
                AllowDrop = true,
                Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right,
            };
            dropPanel.Paint += (s, e) => {
                var color = dropHot ? ColAccentHot : ColPurple;
                using (var pen = new Pen(color, 1.5f) { DashStyle = DashStyle.Dash }) {
                    e.Graphics.DrawRectangle(pen, 1, 1, dropPanel.Width - 3, dropPanel.Height - 3);
                }
                // Little corner brackets, like a cartridge slot.
                using (var pen = new Pen(dropHot ? ColAccentHot : ColGold, 2f)) {
                    int w = dropPanel.Width - 1, h = dropPanel.Height - 1, n = 12;
                    e.Graphics.DrawLines(pen, new[] { new Point(0, n), new Point(0, 0), new Point(n, 0) });
                    e.Graphics.DrawLines(pen, new[] { new Point(w - n, 0), new Point(w, 0), new Point(w, n) });
                    e.Graphics.DrawLines(pen, new[] { new Point(0, h - n), new Point(0, h), new Point(n, h) });
                    e.Graphics.DrawLines(pen, new[] { new Point(w - n, h), new Point(w, h), new Point(w, h - n) });
                }
            };
            dropPanel.DragEnter += OnDragEnter;
            dropPanel.DragLeave += delegate { SetDropHot(false); };
            dropPanel.DragDrop += OnDragDrop;
            dropPanel.MouseEnter += delegate { SetDropHot(true); };
            dropPanel.MouseLeave += delegate { SetDropHot(MouseOverDrop()); };
            dropPanel.Click += delegate { BrowseForRom(); };

            dropLabel = new Label {
                Text = "Drag your ROM here\n(or click to choose it)",
                TextAlign = ContentAlignment.MiddleCenter,
                Font = new Font(SerifFont, 13f),
                Dock = DockStyle.Fill,
                ForeColor = ColText,
                BackColor = Color.Transparent,
            };
            dropLabel.Click += delegate { BrowseForRom(); };
            dropLabel.MouseEnter += delegate { SetDropHot(true); };
            dropLabel.MouseLeave += delegate { SetDropHot(MouseOverDrop()); };
            dropPanel.Controls.Add(dropLabel);

            romStatus = new Label {
                Text = "It must be the USA Nintendo 64 version (.z64, .n64 or .v64).",
                ForeColor = ColDim,
                BackColor = Color.Transparent,
                AutoSize = false,
                Location = new Point(22, 272),
                Size = new Size(580, 20),
                Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right,
            };

            var optionsBox = new Panel {
                BackColor = ColPanel,
                Location = new Point(20, 298),
                Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right,
            };
            optionsBox.Paint += (s, e) => {
                using (var pen = new Pen(ColPurple)) {
                    e.Graphics.DrawRectangle(pen, 0, 0, optionsBox.Width - 1, optionsBox.Height - 1);
                }
            };
            optionsBox.Controls.Add(new Label {
                Text = "OPTIONS",
                ForeColor = ColGold,
                Font = new Font("Segoe UI", 8.25f, FontStyle.Bold),
                AutoSize = true,
                Location = new Point(12, 10),
            });
            int y = 34;
            foreach (var option in Options) {
                option.Box = new CheckBox {
                    Text = option.Label,
                    Checked = GetBool(option.Key, option.Default),
                    ForeColor = ColText,
                    FlatStyle = FlatStyle.Flat,
                    AutoSize = true,
                    Location = new Point(14, y),
                };
                option.Box.FlatAppearance.BorderColor = ColPurple;
                option.Box.FlatAppearance.CheckedBackColor = ColAccent;
                option.Box.FlatAppearance.MouseOverBackColor = ColPanelHot;
                var opt = option;
                option.Box.CheckedChanged += delegate {
                    settings[opt.Key] = opt.Box.Checked ? "1" : "0";
                    SaveSettings();
                };
                optionsBox.Controls.Add(option.Box);
                if (option.Hint != null) {
                    optionsBox.Controls.Add(new Label {
                        Text = option.Hint,
                        ForeColor = ColDim,
                        AutoSize = true,
                        Location = new Point(32, y + 22),
                    });
                    y += 46;
                } else {
                    y += 28;
                }
            }
            optionsBox.Controls.Add(new Label {
                Text = "More options will show up here as the hack grows.",
                ForeColor = ColDim,
                Font = new Font(Font, FontStyle.Italic),
                AutoSize = true,
                Location = new Point(14, y + 4),
            });
            optionsBox.Size = new Size(580, y + 32);

            int buttonsTop = optionsBox.Bottom + 14;
            buildButton = MakeButton("Build", 20, buttonsTop, 200, true);
            buildButton.Click += delegate {
                if (busy) {
                    return;
                }
                if (Find("laugh").Box.Checked) {
                    Laugh.Play();
                }
                header.Shock();
                StartWork(false);
            };
            playButton = MakeButton("Play", 230, buttonsTop, 120, false);
            playButton.Click += delegate { StartWork(true); };
            folderButton = MakeButton("Open folder", 360, buttonsTop, 140, false);
            folderButton.Click += delegate { OpenOutputFolder(); };

            statusLabel = new Label {
                Text = "",
                ForeColor = ColText,
                BackColor = Color.Transparent,
                AutoSize = false,
                Location = new Point(20, buttonsTop + 52),
                Size = new Size(580, 20),
                Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right,
            };
            progress = new PulseBar {
                Location = new Point(20, buttonsTop + 76),
                Size = new Size(580, 4),
                Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right,
            };
            logBox = new TextBox {
                Multiline = true,
                ReadOnly = true,
                ScrollBars = ScrollBars.Vertical,
                WordWrap = true,
                BackColor = ColPanel,
                ForeColor = ColDim,
                BorderStyle = BorderStyle.None,
                Font = new Font("Consolas", 9f),
                Location = new Point(20, buttonsTop + 90),
                Size = new Size(580, ClientSize.Height - (buttonsTop + 90) - 16),
                Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right,
            };

            Controls.AddRange(new Control[] {
                header, dropPanel, romStatus, optionsBox, buildButton, playButton, folderButton, statusLabel, progress,
                logBox,
            });
        }

        bool MouseOverDrop()
        {
            return dropPanel.ClientRectangle.Contains(dropPanel.PointToClient(MousePosition));
        }

        void SetDropHot(bool hot)
        {
            if (dropHot != hot) {
                dropHot = hot;
                dropPanel.BackColor = hot ? ColPanelHot : ColPanel;
                dropPanel.Invalidate();
            }
        }

        Button MakeButton(string text, int x, int y, int width, bool primary)
        {
            var b = new Button {
                Text = text,
                Location = new Point(x, y),
                Size = new Size(width, 42),
                FlatStyle = FlatStyle.Flat,
                BackColor = primary ? ColAccent : ColPanel,
                ForeColor = ColText,
                Font = primary ? new Font(SerifFont, 13f, FontStyle.Bold) : new Font("Segoe UI", 10f),
                Cursor = Cursors.Hand,
            };
            b.FlatAppearance.BorderColor = primary ? ColAccentHot : ColPurple;
            b.FlatAppearance.MouseOverBackColor = primary ? ColAccentHot : ColPanelHot;
            b.FlatAppearance.MouseDownBackColor = primary ? Color.FromArgb(110, 10, 18) : ColBack;
            return b;
        }

        // ---------------------------------------------------------------------------------------------------------
        // ROM

        void OnDragEnter(object sender, DragEventArgs e)
        {
            e.Effect = (!busy && e.Data.GetDataPresent(DataFormats.FileDrop)) ? DragDropEffects.Copy : DragDropEffects.None;
            SetDropHot(e.Effect != DragDropEffects.None);
        }

        void OnDragDrop(object sender, DragEventArgs e)
        {
            SetDropHot(false);
            var files = e.Data.GetData(DataFormats.FileDrop) as string[];
            if (files != null && files.Length > 0) {
                SetRom(files[0]);
            }
        }

        void BrowseForRom()
        {
            if (busy) {
                return;
            }
            using (var dialog = new OpenFileDialog()) {
                dialog.Title = "Select your Majora's Mask (USA) N64 ROM";
                dialog.Filter = "N64 ROMs (*.z64;*.n64;*.v64)|*.z64;*.n64;*.v64|All files (*.*)|*.*";
                if (dialog.ShowDialog(this) == DialogResult.OK) {
                    SetRom(dialog.FileName);
                }
            }
        }

        void SetRom(string path)
        {
            var info = new FileInfo(path);
            if (!info.Exists) {
                return;
            }
            romPath = path;
            settings["rom"] = path;
            SaveSettings();
            dropLabel.Text = info.Name;

            if (info.Length != RomSize) {
                romStatus.ForeColor = ColBad;
                romStatus.Text = "This doesn't look like a Majora's Mask ROM (wrong size). The build will tell you for sure.";
            } else if (info.Extension.Equals(".z64", StringComparison.OrdinalIgnoreCase) && Md5(path) == RetailMd5) {
                romStatus.ForeColor = ColGood;
                romStatus.Text = "Majora's Mask (USA). Good to go.";
            } else {
                romStatus.ForeColor = ColDim;
                romStatus.Text = "The build checks this ROM and converts .n64/.v64 automatically.";
            }
        }

        static string Md5(string path)
        {
            using (var md5 = MD5.Create())
            using (var stream = File.OpenRead(path)) {
                return BitConverter.ToString(md5.ComputeHash(stream)).Replace("-", "").ToLowerInvariant();
            }
        }

        // ---------------------------------------------------------------------------------------------------------
        // Work

        void StartWork(bool play)
        {
            if (busy) {
                return;
            }
            SetBusy(true);
            logBox.Clear();
            var thread = new Thread(() => {
                bool ok = false;
                try {
                    ok = play ? DoPlay() : DoBuild();
                } catch (Exception ex) {
                    Log("Error: " + ex.Message);
                }
                BeginInvoke((Action)(() => {
                    SetBusy(false);
                    if (!ok) {
                        Status("You've met with a terrible fate. The build stopped; see the messages below.", ColBad);
                    }
                }));
            });
            thread.IsBackground = true;
            thread.Start();
        }

        bool DoBuild()
        {
            string distro = EnsureUbuntu();
            if (distro == null) {
                return false;
            }

            Status("Getting the newest version of the project...", ColText);
            string script =
                "if [ -d " + ProjectFolder + "/.git ]; then " +
                "git -C " + ProjectFolder + " pull --ff-only || echo \"Couldn't update; building the version you have.\"; " +
                "else git clone " + RepoUrl + " " + ProjectFolder + "; fi";
            if (Wsl(distro, false, "bash", "-c", script) != 0) {
                Log("Downloading the project failed. Check your internet connection and try again.");
                return false;
            }

            string home = WslOutput(distro, "sh", "-c", "cd ~ && pwd");
            if (string.IsNullOrEmpty(home)) {
                Log("Couldn't find your Ubuntu home folder.");
                return false;
            }
            string ben = home + "/" + ProjectFolder + "/ben";

            Status("Installing the build tools (first time only)...", ColText);
            if (Wsl(distro, true, "bash", ben, "packages") != 0) {
                Log("Installing the build tools failed (see above).");
                return false;
            }

            foreach (var option in Options) {
                if (option.OptKey != null) {
                    Wsl(distro, false, "bash", ben, "config", "OPT_" + option.OptKey, IsChecked(option) ? "1" : "0");
                }
            }

            var args = new List<string> { "bash", ben, "setup" };
            if (romPath != null && File.Exists(romPath)) {
                string wslRom = WslOutput(distro, "wslpath", "-a", "-u", romPath);
                if (string.IsNullOrEmpty(wslRom)) {
                    Log("Couldn't hand the ROM to Ubuntu: " + romPath);
                    return false;
                }
                args.Add(wslRom);
            }
            Status("Building BEN DROWNED. The first build takes 10 to 30 minutes...", ColText);
            if (Wsl(distro, false, args.ToArray()) != 0) {
                return false;
            }

            if (IsChecked(Find("makePatch"))) {
                Status("Making the patch...", ColText);
                if (Wsl(distro, false, "bash", ben, "patch") != 0) {
                    return false;
                }
            }

            Status("Dawn of a new day. Your ROM is BEN-DROWNED-latest.z64 in " + outputFolder, ColGood);
            Log("");
            Log("Done. Press Play, or open BEN-DROWNED-latest.z64 from " + outputFolder + " in your emulator.");
            if (IsChecked(Find("openFolder"))) {
                BeginInvoke((Action)OpenOutputFolder);
            }
            return true;
        }

        bool DoPlay()
        {
            string distro = EnsureUbuntu();
            if (distro == null) {
                return false;
            }
            Status("Starting the emulator...", ColText);
            string home = WslOutput(distro, "sh", "-c", "cd ~ && pwd");
            if (Wsl(distro, false, "bash", home + "/" + ProjectFolder + "/ben", "run") != 0) {
                return false;
            }
            Status("", ColText);
            return true;
        }

        // Returns the Ubuntu distribution to use, or null after explaining what the user needs to do.
        string EnsureUbuntu()
        {
            Status("Checking WSL (the Linux environment the build runs in)...", ColText);
            if (WslExe() == null) {
                Log("This version of Windows has no WSL. Windows 10 version 2004 or newer, or Windows 11, is required.");
                return null;
            }

            string distro = FindUbuntu();
            if (distro == null) {
                var answer = (DialogResult)Invoke((Func<DialogResult>)(() => MessageBox.Show(this,
                    "The build runs inside Ubuntu (WSL), which isn't installed yet.\n\n" +
                    "Install it now? Windows will ask for administrator permission, and you'll need to restart.",
                    "Install Ubuntu", MessageBoxButtons.YesNo, MessageBoxIcon.Question)));
                if (answer != DialogResult.Yes) {
                    Log("Ubuntu (WSL) is needed to build. Press Build again when you're ready to install it.");
                    return null;
                }
                try {
                    var install = Process.Start(new ProcessStartInfo(WslExe(), "--install -d Ubuntu") {
                        UseShellExecute = true,
                        Verb = "runas",
                    });
                    install.WaitForExit();
                } catch (Exception) {
                    Log("Administrator permission was refused, so WSL could not be installed.");
                    return null;
                }
                Log("Almost there:");
                Log("  1. Restart your PC.");
                Log("  2. Open \"Ubuntu\" from the Start menu once and choose a username and password.");
                Log("  3. Open this app again and press Build.");
                return null;
            }

            if (Wsl(distro, false, "true") != 0) {
                Log("Ubuntu (" + distro + ") is installed but didn't start. Open Ubuntu from the Start menu once to " +
                    "finish its setup, then press Build again.");
                return null;
            }
            Log("Using " + distro);
            return distro;
        }

        static string WslExe()
        {
            string windows = Environment.GetFolderPath(Environment.SpecialFolder.Windows);
            foreach (var dir in new[] { "System32", "Sysnative" }) {
                string path = Path.Combine(Path.Combine(windows, dir), "wsl.exe");
                if (File.Exists(path)) {
                    return path;
                }
            }
            return null;
        }

        string FindUbuntu()
        {
            string list = RunCapture(WslExe(), "-l -q");
            if (list == null) {
                return null;
            }
            foreach (var raw in list.Split('\n')) {
                string name = raw.Replace("\0", "").Trim();
                if (name.StartsWith("Ubuntu", StringComparison.OrdinalIgnoreCase)) {
                    return name;
                }
            }
            return null;
        }

        // Runs a command inside Ubuntu (in your home folder) and streams its output to the log. Returns the exit code.
        int Wsl(string distro, bool asRoot, params string[] command)
        {
            var info = WslStartInfo(distro, asRoot, command);
            using (var process = new Process { StartInfo = info }) {
                process.OutputDataReceived += (s, e) => { if (e.Data != null) Log(e.Data); };
                process.ErrorDataReceived += (s, e) => { if (e.Data != null) Log(e.Data); };
                process.Start();
                process.BeginOutputReadLine();
                process.BeginErrorReadLine();
                process.WaitForExit();
                return process.ExitCode;
            }
        }

        // Runs a command inside Ubuntu and returns the first line it prints, or null if it failed.
        string WslOutput(string distro, params string[] command)
        {
            var info = WslStartInfo(distro, false, command);
            using (var process = new Process { StartInfo = info }) {
                process.Start();
                string output = process.StandardOutput.ReadToEnd();
                process.StandardError.ReadToEnd();
                process.WaitForExit();
                if (process.ExitCode != 0) {
                    return null;
                }
                return output.Split('\n')[0].Trim();
            }
        }

        static ProcessStartInfo WslStartInfo(string distro, bool asRoot, string[] command)
        {
            var args = new StringBuilder();
            args.Append("-d ").Append(Quote(distro));
            if (asRoot) {
                args.Append(" -u root");
            }
            args.Append(" --cd ~ --exec");
            foreach (var part in command) {
                args.Append(' ').Append(Quote(part));
            }
            var info = new ProcessStartInfo(WslExe(), args.ToString()) {
                UseShellExecute = false,
                CreateNoWindow = true,
                RedirectStandardOutput = true,
                RedirectStandardError = true,
                StandardOutputEncoding = Encoding.UTF8,
                StandardErrorEncoding = Encoding.UTF8,
            };
            info.EnvironmentVariables["WSL_UTF8"] = "1";
            return info;
        }

        // Runs a Windows program and returns everything it printed (UTF-16 or UTF-8), or null if it failed.
        static string RunCapture(string exe, string arguments)
        {
            var info = new ProcessStartInfo(exe, arguments) {
                UseShellExecute = false,
                CreateNoWindow = true,
                RedirectStandardOutput = true,
                RedirectStandardError = true,
            };
            info.EnvironmentVariables["WSL_UTF8"] = "1";
            try {
                using (var process = Process.Start(info))
                using (var buffer = new MemoryStream()) {
                    process.StandardOutput.BaseStream.CopyTo(buffer);
                    process.StandardError.ReadToEnd();
                    process.WaitForExit();
                    if (process.ExitCode != 0) {
                        return null;
                    }
                    byte[] bytes = buffer.ToArray();
                    bool utf16 = bytes.Length > 1 && bytes[1] == 0;
                    return (utf16 ? Encoding.Unicode : Encoding.UTF8).GetString(bytes);
                }
            } catch (Exception) {
                return null;
            }
        }

        // Windows command-line quoting for one argument.
        static string Quote(string arg)
        {
            if (arg.Length > 0 && arg.IndexOfAny(new[] { ' ', '\t', '"' }) < 0) {
                return arg;
            }
            var sb = new StringBuilder("\"");
            int backslashes = 0;
            foreach (char c in arg) {
                if (c == '\\') {
                    backslashes++;
                    continue;
                }
                if (c == '"') {
                    sb.Append('\\', backslashes * 2 + 1);
                } else {
                    sb.Append('\\', backslashes);
                }
                backslashes = 0;
                sb.Append(c);
            }
            sb.Append('\\', backslashes * 2);
            return sb.Append('"').ToString();
        }

        // ---------------------------------------------------------------------------------------------------------
        // UI helpers

        void SetBusy(bool value)
        {
            busy = value;
            buildButton.Enabled = !value;
            playButton.Enabled = !value;
            progress.Running = value;
            foreach (var option in Options) {
                option.Box.Enabled = !value;
            }
        }

        void Status(string text, Color color)
        {
            if (InvokeRequired) {
                BeginInvoke((Action)(() => Status(text, color)));
                return;
            }
            statusLabel.Text = text;
            statusLabel.ForeColor = color;
            if (text.Length > 0) {
                Log("== " + text);
            }
        }

        void Log(string line)
        {
            pendingLog.Enqueue(line);
        }

        void FlushLog()
        {
            if (pendingLog.IsEmpty) {
                return;
            }
            var sb = new StringBuilder();
            string line;
            while (pendingLog.TryDequeue(out line)) {
                sb.Append(line).Append("\r\n");
            }
            logBox.AppendText(sb.ToString());
            // Keep the log from growing without bound during a full build.
            if (logBox.TextLength > 200000) {
                logBox.Text = logBox.Text.Substring(logBox.TextLength - 100000);
                logBox.SelectionStart = logBox.TextLength;
                logBox.ScrollToCaret();
            }
        }

        void OpenOutputFolder()
        {
            Directory.CreateDirectory(outputFolder);
            Process.Start("explorer.exe", Quote(outputFolder));
        }

        BuildOption Find(string key)
        {
            return Options.Find(o => o.Key == key);
        }

        bool IsChecked(BuildOption option)
        {
            return (bool)Invoke((Func<bool>)(() => option.Box.Checked));
        }

        // ---------------------------------------------------------------------------------------------------------
        // Settings (%APPDATA%\BEN-DROWNED\builder.ini, key=value)

        void LoadSettings()
        {
            try {
                if (!File.Exists(settingsPath)) {
                    return;
                }
                foreach (var line in File.ReadAllLines(settingsPath)) {
                    int eq = line.IndexOf('=');
                    if (eq > 0) {
                        settings[line.Substring(0, eq)] = line.Substring(eq + 1);
                    }
                }
            } catch (Exception) {
                // Bad or unreadable settings just mean defaults.
            }
        }

        void SaveSettings()
        {
            try {
                Directory.CreateDirectory(Path.GetDirectoryName(settingsPath));
                var lines = new List<string>();
                foreach (var pair in settings) {
                    lines.Add(pair.Key + "=" + pair.Value);
                }
                File.WriteAllLines(settingsPath, lines.ToArray());
            } catch (Exception) {
                // Not being able to remember settings isn't worth interrupting anyone over.
            }
        }

        bool GetBool(string key, bool fallback)
        {
            string value;
            return settings.TryGetValue(key, out value) ? value == "1" : fallback;
        }

        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new BuilderForm());
        }
    }
}
