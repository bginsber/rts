using UnrealBuildTool;

public class RTS : ModuleRules
{
	public RTS(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore",
			"HeadMountedDisplay",
			"EnhancedInput",
			"NetCore",
			"OnlineSubsystem",
			"OnlineSubsystemUtils",
			"Landscape",
			"Foliage"
		});

		if (Target.Type == TargetType.Editor)
		{
			PublicDependencyModuleNames.Add("LandscapeEditor");
		}

		PrivateDependencyModuleNames.AddRange(new string[] {  });
	}
}