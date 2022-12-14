// Fill out your copyright notice in the Description page of Project Settings.


#include "LumafuseDisplayRenderer.h"

// Sets default values
ALumafuseDisplayRenderer::ALumafuseDisplayRenderer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ALumafuseDisplayRenderer::BeginPlay()
{
	Super::BeginPlay();
	
} 

// Called every frame
void ALumafuseDisplayRenderer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ALumafuseDisplayRenderer::CopyTextureQuadrant(const FTexture2DRHIRef& SourceTexture, FTexture2DRHIRef& DestinationTexture, FIntPoint QuadrantPosition)
{
	FRHICommandListImmediate& RHICmdList = FRHICommandListExecutor::GetImmediateCommandList();

	IRendererModule* RendererModule = &FModuleManager::GetModuleChecked<IRendererModule>("Renderer");
	
	FRHIRenderPassInfo RPInfo(DestinationTexture, ERenderTargetActions::Load_Store);

	RHICmdList.BeginRenderPass(RPInfo, TEXT("CopyBackbuffer"));

	{
		RHICmdList.SetViewport(0, 0, 0.0f, SourceTexture->GetSizeX(), SourceTexture->GetSizeY(), 1.0f);

		FGraphicsPipelineStateInitializer GraphicsPSOInit;
		RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
		GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
		GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
		GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();

		// New engine version...
		FGlobalShaderMap* ShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
		TShaderMapRef<FScreenVS> VertexShader(ShaderMap);
		TShaderMapRef<FScreenPS> PixelShader(ShaderMap);

		GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GFilterVertexDeclaration.VertexDeclarationRHI;
		GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
		GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();

		GraphicsPSOInit.PrimitiveType = PT_TriangleList;

		SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);
		
		/*
		if(DestinationTexture->GetSizeX() != SourceTexture->GetSizeX() || DestinationTexture->GetSizeY() != SourceTexture->GetSizeY())
		{
			PixelShader->SetParameters(RHICmdList, TStaticSamplerState<SF_Bilinear>::GetRHI(), SourceTexture);
		}
		else
		{
			PixelShader->SetParameters(RHICmdList, TStaticSamplerState<SF_Point>::GetRHI(), SourceTexture);
		}
		*/

		PixelShader->SetParameters(RHICmdList, TStaticSamplerState<SF_Point>::GetRHI(), SourceTexture);

		RendererModule->DrawRectangle(RHICmdList, QuadrantPosition.X / -2, QuadrantPosition.Y / -2, // Dest X, Y (Converting to positive coordinates of source texture)
		                              DestinationTexture->GetSizeX(),  // Dest Width
		                              DestinationTexture->GetSizeY(),  // Dest Height
		                              0, 0,                            // Source U, V
		                              1, 1,                            // Source USize, VSize
		                              DestinationTexture->GetSizeXY(), // Target buffer size
		                              FIntPoint(1, 1),                 // Source texture size
		                              VertexShader, EDRF_Default);
	}

	RHICmdList.EndRenderPass();
}

void ALumafuseDisplayRenderer::GetQuadrantPixelBufferFromRenderTargetThreadSafe(UTextureRenderTarget2D* TextureRenderTarget, FIntPoint QuadrantPosition,
                                                                        TArray<uint8>& Buffer, int32 CompressionQuality)
{
	ENQUEUE_RENDER_COMMAND(ReadSurfaceCommand)([this, TextureRenderTarget, &Buffer, CompressionQuality, QuadrantPosition](FRHICommandListImmediate& RHICmdList)
	{
		// A frame is supplied so immediately read its data and compress it with JPEG compression.
		FTexture2DRHIRef Texture2DRHI = (TextureRenderTarget->Resource && TextureRenderTarget->Resource->TextureRHI) ? TextureRenderTarget->Resource->TextureRHI->GetTexture2D() : nullptr;

		if (!Texture2DRHI)
		{
			UE_LOG(LogTemp, Error, TEXT("Attempting freeze frame with texture %s with no texture 2D RHI"), *TextureRenderTarget->GetName());
			return;
		}

		//Initialize Width and Height values to the size of the source texture
		uint32 Width = Texture2DRHI->GetSizeX();
		uint32 Height = Texture2DRHI->GetSizeY();
		const FIntPoint QuadrantSize = FIntPoint(Width / 2, Height / 2);
		
		// Create empty texture
		FRHIResourceCreateInfo CreateInfo(TEXT("FreezeFrameTexture"));
		FTexture2DRHIRef DestTexture = GDynamicRHI->RHICreateTexture2D(QuadrantSize.X, QuadrantSize.Y, EPixelFormat::PF_B8G8R8A8, 1, 1, TexCreate_RenderTargetable, ERHIAccess::Present, CreateInfo);

		// Copy freeze frame texture to empty texture
		CopyTextureQuadrant(Texture2DRHI, DestTexture, QuadrantPosition);

		// Create the empty pixel surface to read the texture into
		TArray<FColor> Data;
		FIntRect Rect(0, 0, QuadrantSize.X, QuadrantSize.Y);
		RHICmdList.ReadSurfaceData(DestTexture, Rect, Data, FReadSurfaceDataFlags());

		// Compress the surface data and set the byte data to the buffer
		CompressPixelsToBuffer(Data, Buffer, QuadrantSize.X, QuadrantSize.Y, CompressionQuality);
	});
}

void ALumafuseDisplayRenderer::CompressPixelsToBuffer(TArray<FColor>& SurfaceData, TArray<uint8>& Buffer, int32 SizeX,
                                                      int32 SizeY, int32 CompressionQuality)
{
	// Compress the surface data and set the byte data to the buffer
	IImageWrapperModule& ImageWrapperModule = FModuleManager::GetModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));
	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG);

	bool Success = ImageWrapper->SetRaw(SurfaceData.GetData(), SurfaceData.GetAllocatedSize(), SizeX, SizeY, ERGBFormat::BGRA, 8);
	if (Success)
	{
		Buffer = ImageWrapper->GetCompressed(CompressionQuality);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to compress image"));
	}
}

