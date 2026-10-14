// The Happy Mask Salesman's laugh for the Build button.
//
// The real laugh is laugh.wav, embedded in the exe. If it's missing (for example in a build made without it), a
// synthesized one plays instead: a breathy "hee hee hee... haaa" made from a glottal pulse run through vowel formants,
// doubled an octave down and given a long echo.
//
// To use a different laugh, put a file named laugh.wav next to BEN-Builder.exe or in %APPDATA%\BEN-DROWNED. The app
// plays that instead.

using System;
using System.IO;
using System.Media;
using System.Text;

namespace BenDrowned
{
    static class Laugh
    {
        const int Rate = 22050;

        static byte[] cached;
        static SoundPlayer player;

        // Plays the laugh without waiting for it to finish. Never throws: a missing sound isn't worth an error.
        public static void Play()
        {
            try {
                if (player == null) {
                    string custom = FindCustom();
                    byte[] wav = custom != null ? File.ReadAllBytes(custom) : (Embedded() ?? Wav());
                    player = new SoundPlayer(new MemoryStream(wav));
                    player.Load();
                }
                player.Play();
            } catch (Exception) {
                player = null;
            }
        }

        static byte[] Embedded()
        {
            using (var stream = typeof(Laugh).Assembly.GetManifestResourceStream("BenDrowned.laugh.wav")) {
                if (stream == null) {
                    return null;
                }
                var copy = new MemoryStream();
                stream.CopyTo(copy);
                return copy.ToArray();
            }
        }

        static string FindCustom()
        {
            string[] places = {
                Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "laugh.wav"),
                Path.Combine(Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData),
                                          "BEN-DROWNED"),
                             "laugh.wav"),
            };
            foreach (string path in places) {
                if (File.Exists(path)) {
                    return path;
                }
            }
            return null;
        }

        // One "hee" or "haa": when it starts and how long it lasts (seconds), its pitch at the start and end (Hz) and
        // its vowel.
        struct Syllable
        {
            public double Start, Length, PitchFrom, PitchTo;
            public Vowel Vowel;

            public Syllable(double start, double length, double from, double to, Vowel vowel)
            {
                Start = start;
                Length = length;
                PitchFrom = from;
                PitchTo = to;
                Vowel = vowel;
            }
        }

        // The first three formant frequencies of a vowel (Hz).
        struct Vowel
        {
            public double F1, F2, F3;

            public Vowel(double f1, double f2, double f3)
            {
                F1 = f1;
                F2 = f2;
                F3 = f3;
            }
        }

        static readonly Vowel Ee = new Vowel(300, 2250, 2950);
        static readonly Vowel Eh = new Vowel(520, 1850, 2550);
        static readonly Vowel Ah = new Vowel(760, 1250, 2500);

        // High and quick at first, then slower and lower, ending in one long drawn-out "haaa".
        static readonly Syllable[] Pattern = {
            new Syllable(0.00, 0.12, 310, 290, Ee),
            new Syllable(0.15, 0.12, 330, 305, Ee),
            new Syllable(0.30, 0.12, 320, 295, Ee),
            new Syllable(0.45, 0.13, 300, 270, Eh),
            new Syllable(0.62, 0.14, 275, 245, Eh),
            new Syllable(0.82, 0.16, 250, 220, Ah),
            new Syllable(1.06, 0.18, 225, 195, Ah),
            new Syllable(1.36, 0.70, 205, 140, Ah),
        };

        // Builds the laugh as a 16-bit mono WAV file.
        public static byte[] Wav()
        {
            if (cached != null) {
                return cached;
            }
            var random = new Random(1987);
            int length = (int)(Rate * 3.6);
            var mix = new double[length];

            // The voice, and a second, deeper voice an octave down and a little behind it. Each syllable is brought to
            // the same loudness so the bright "hee"s aren't drowned out by the open "haa"s.
            foreach (var syllable in Pattern) {
                var one = new double[length];
                Voice(one, syllable, 1.0, 1.0, 0.0, 1.0, random);
                Voice(one, syllable, 0.5, 0.82, 0.018, 0.6, random);
                double loudest = 1e-9;
                foreach (double v in one) {
                    loudest = Math.Max(loudest, Math.Abs(v));
                }
                for (int i = 0; i < length; i++) {
                    mix[i] += one[i] / loudest;
                }
            }

            // A low, beating hum under it.
            for (int i = 0; i < length; i++) {
                double t = (double)i / Rate;
                double swell = Math.Sin(Math.PI * Math.Min(1.0, t / 3.0));
                mix[i] += 0.05 * swell * (Math.Sin(2 * Math.PI * 55 * t) + Math.Sin(2 * Math.PI * 58.3 * t));
            }

            // A dark echo, as if the laugh fills a big empty room.
            var output = new double[length];
            int delay = (int)(Rate * 0.21);
            double low = 0;
            for (int i = 0; i < length; i++) {
                double echo = 0;
                if (i >= delay) {
                    low += 0.35 * (output[i - delay] - low);
                    echo = 0.42 * low;
                }
                output[i] = mix[i] + echo;
            }

            double peak = 1e-9;
            foreach (double v in output) {
                peak = Math.Max(peak, Math.Abs(v));
            }
            var samples = new short[length];
            for (int i = 0; i < length; i++) {
                double fadeOut = Math.Min(1.0, (length - i) / (Rate * 0.4));
                samples[i] = (short)(output[i] / peak * 0.8 * fadeOut * short.MaxValue);
            }
            cached = ToWav(samples);
            return cached;
        }

        // Adds one syllable to `mix`. `pitch` and `formants` scale the voice (0.5 is an octave down; below 1 sounds like
        // a bigger throat), `lag` delays it in seconds and `gain` sets its loudness.
        static void Voice(double[] mix, Syllable s, double pitch, double formants, double lag, double gain,
                          Random random)
        {
            int start = (int)((s.Start + lag) * Rate);
            int count = (int)(s.Length * Rate);
            var f1 = new Resonator();
            var f2 = new Resonator();
            var f3 = new Resonator();
            double phase = 0;
            double lastPulse = 0;
            double jitter = 1;
            double noiseLow = 0;

            int ringOut = Rate / 20; // let the formants die away instead of cutting them off
            for (int n = 0; n < count + ringOut && start + n < mix.Length; n++) {
                double x = Math.Min(1.0, (double)n / count);
                double t = (double)n / Rate;

                // The vowel opens out of a breathy "h".
                double open = Math.Min(1.0, t / 0.04);
                f1.Set(Lerp(450, s.Vowel.F1, open) * formants, 90, Rate);
                f2.Set(Lerp(1600, s.Vowel.F2, open) * formants, 120, Rate);
                f3.Set(s.Vowel.F3 * formants, 180, Rate);

                double f0 = Lerp(s.PitchFrom, s.PitchTo, x) * pitch;
                f0 *= 1 + 0.03 * Math.Sin(2 * Math.PI * 5.5 * t) * x; // a shaky vibrato as the syllable trails off
                phase += f0 * jitter / Rate;
                if (phase >= 1) {
                    phase -= 1;
                    jitter = 1 + (random.NextDouble() - 0.5) * 0.04;
                }
                double pulse = GlottalPulse(phase);
                double voiced = pulse - lastPulse; // the mouth radiates the pulse's slope
                lastPulse = pulse;

                // Breath noise, slightly darkened.
                noiseLow += 0.5 * ((random.NextDouble() * 2 - 1) - noiseLow);

                double attack = Math.Min(1.0, t / 0.03);
                double voicing = Smooth((t - 0.015) / 0.03);
                double decay = Math.Exp(-3.2 * Math.Max(0, x - 0.35) / 0.65);
                double release = Math.Max(0.0, Math.Min(1.0, (count - n) / (Rate * 0.03)));
                double breath = 0.010 + 0.035 * (1 - voicing) + 0.012 * x;

                double source = attack * decay * release * (voicing * voiced * 4.0 + breath * noiseLow);
                double y = f3.Run(f2.Run(f1.Run(source)));
                mix[start + n] += gain * y;
            }
        }

        // A Rosenberg glottal pulse: the vocal folds opening, closing and resting over one period (phase 0..1).
        static double GlottalPulse(double phase)
        {
            const double opening = 0.45, closing = 0.09;
            if (phase < opening) {
                return 0.5 * (1 - Math.Cos(Math.PI * phase / opening));
            }
            if (phase < opening + closing) {
                return Math.Cos(Math.PI * (phase - opening) / (2 * closing));
            }
            return 0;
        }

        // A two-pole resonator (one formant), as in Klatt's speech synthesizer.
        sealed class Resonator
        {
            double a, b, c, y1, y2;

            public void Set(double frequency, double bandwidth, int rate)
            {
                double r = Math.Exp(-Math.PI * bandwidth / rate);
                c = -r * r;
                b = 2 * r * Math.Cos(2 * Math.PI * frequency / rate);
                a = 1 - b - c;
            }

            public double Run(double x)
            {
                double y = a * x + b * y1 + c * y2;
                y2 = y1;
                y1 = y;
                return y;
            }
        }

        static double Lerp(double from, double to, double x)
        {
            return from + (to - from) * x;
        }

        static double Smooth(double x)
        {
            x = Math.Max(0, Math.Min(1, x));
            return x * x * (3 - 2 * x);
        }

        static byte[] ToWav(short[] samples)
        {
            using (var stream = new MemoryStream())
            using (var w = new BinaryWriter(stream)) {
                w.Write(Encoding.ASCII.GetBytes("RIFF"));
                w.Write(36 + samples.Length * 2);
                w.Write(Encoding.ASCII.GetBytes("WAVEfmt "));
                w.Write(16);
                w.Write((short)1); // PCM
                w.Write((short)1); // mono
                w.Write(Rate);
                w.Write(Rate * 2);
                w.Write((short)2);
                w.Write((short)16);
                w.Write(Encoding.ASCII.GetBytes("data"));
                w.Write(samples.Length * 2);
                foreach (short s in samples) {
                    w.Write(s);
                }
                w.Flush();
                return stream.ToArray();
            }
        }
    }
}
