using UnrealBuildTool;
using System.IO;

public class LowEntryCompression : ModuleRules
{
	public LowEntryCompression(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
			new string[] {
                Path.Combine(ModuleDirectory, "Public/Module"),
                Path.Combine(ModuleDirectory, "Public/Classes"),
                Path.Combine(ModuleDirectory, "Public/Enums"),
			}
		);

		PrivateIncludePaths.AddRange(
			new string[] {
                Path.Combine(ModuleDirectory, "Private/Module"),
                Path.Combine(ModuleDirectory, "Private/Classes"),
			}
		);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Engine",
				"Core",
				"CoreUObject",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
			}
		);

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
		);
	}
}
