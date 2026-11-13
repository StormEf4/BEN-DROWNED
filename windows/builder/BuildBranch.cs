// Which branch of the project the builder builds. Empty: whatever the project folder already has (the default branch
// when it's first downloaded). The CI build (.github/workflows/builder.yml) writes the branch it was built from here, so
// a builder made from a pull request builds that pull request's version, and the released one builds the default branch.

namespace BenDrowned
{
    static class BuildBranch
    {
        public const string Name = "";
    }
}
