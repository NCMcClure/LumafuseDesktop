// Copyright 2017-2021 S.C. Pug Life Studio S.R.L. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class MultiTask2 : ModuleRules
{
    public MultiTask2(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public/BlueprintLibraries"));
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public/Tasks"));
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));

        PublicDependencyModuleNames.AddRange(
            new[]
            {
                "Core",
                "ProceduralMeshComponent",
                "DynamicMesh",
                "GeometricObjects",
                "GeometryAlgorithms"

                // ... add other public dependencies that you statically link with here ...
            }
        );


        PrivateDependencyModuleNames.AddRange(
            new[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "RHI",
                "RenderCore",
                "ImageWriteQueue",
                "ImageWrapper",
                "HTTP"
                // ... add private dependencies that you statically link with here ...	
            }
        );

        if (Target.Type == TargetRules.TargetType.Editor)
            PrivateDependencyModuleNames.AddRange(
                new[]
                {
                    "UnrealEd"
                }
            );


        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
                // ... add any modules that your module loads dynamically here ...
            }
        );
    }
}