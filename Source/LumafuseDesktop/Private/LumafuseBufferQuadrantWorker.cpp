// Fill out your copyright notice in the Description page of Project Settings.


#include "LumafuseBufferQuadrantWorker.h"

void ULumafuseBufferQuadrantWorker::CopyTextureBlock(const FTexture2DRHIRef& SourceTexture,
	FTexture2DRHIRef& DestinationTexture, FIntPoint BlockPosition, FIntPoint GridLayout)
{
	FRHICommandListImmediate& RHICmdList = FRHICommandListExecutor::GetImmediateCommandList();

	IRendererModule* RendererModule = &FModuleManager::GetModuleChecked<IRendererModule>("Renderer");
	
	FRHIRenderPassInfo RPInfo(DestinationTexture, ERenderTargetActions::Load_Store);

	RHICmdList.BeginRenderPass(RPInfo, TEXT("CopyBackbuffer"));

	int32 SourceWidth = SourceTexture->GetSizeX();
	int32 SourceHeight = SourceTexture->GetSizeY();

	int32 DestWidth = DestinationTexture->GetSizeX();
	int32 DestHeight = DestinationTexture->GetSizeY();
	FIntPoint DestSize = FIntPoint(DestWidth, DestHeight);

	{
		RHICmdList.SetViewport(0, 0, 0.0f, SourceWidth, SourceHeight, 1.0f);

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

		RendererModule->DrawRectangle(RHICmdList, -((DestWidth * BlockPosition.X) / GridLayout.X), -((DestHeight * BlockPosition.Y) / GridLayout.Y), // Dest X, Y (Converting to positive relative coordinates of new dest texture)
		                              DestWidth,                          // Dest Width
		                              DestHeight,                         // Dest Height
		                              0, 0,                             // Source U, V
		                              1, 1,                          // Source USize, VSize
		                              DestSize,                           // Target buffer size
		                              FIntPoint(1, 1),             // Source texture size
		                              VertexShader, EDRF_Default);
	}

	RHICmdList.EndRenderPass();	
	
}

void ULumafuseBufferQuadrantWorker::GetPixelBufferBlockFromRenderTargetThreadSafe(UTextureRenderTarget2D* TextureRenderTarget, FIntPoint BlockPosition, FIntPoint GridLayout,
																		TArray<uint8>& Buffer, int32 CompressionQuality)
{
	ENQUEUE_RENDER_COMMAND(ReadSurfaceCommand)([this, TextureRenderTarget, &Buffer, CompressionQuality, BlockPosition, GridLayout](FRHICommandListImmediate& RHICmdList)
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
	const FIntPoint BlockSize = FIntPoint(Width / GridLayout.X, Height / GridLayout.Y);
		
	// Create empty texture
	FRHIResourceCreateInfo CreateInfo(TEXT("FreezeFrameTexture"));
	FTexture2DRHIRef DestTexture = GDynamicRHI->RHICreateTexture2D(BlockSize.X, BlockSize.Y, EPixelFormat::PF_B8G8R8A8, 1, 1, TexCreate_RenderTargetable, ERHIAccess::Present, CreateInfo);

	// Copy freeze frame texture to empty texture
	CopyTextureBlock(Texture2DRHI, DestTexture, BlockPosition, GridLayout);

	// Create the empty pixel surface to read the texture into
	TArray<FColor> Data;
	FIntRect Rect(0, 0, BlockSize.X, BlockSize.Y);
	RHICmdList.ReadSurfaceData(DestTexture, Rect, Data, FReadSurfaceDataFlags());

	// Compress the surface data and set the byte data to the buffer
	CompressPixelsToBuffer(Data, Buffer, BlockSize.X, BlockSize.Y, CompressionQuality);
});
}

void ULumafuseBufferQuadrantWorker::GetPixelBufferFromRenderTargetThreadSafe(UTextureRenderTarget2D* TextureRenderTarget,
																		TArray<uint8>& Buffer, int32 CompressionQuality)
{
	ENQUEUE_RENDER_COMMAND(ReadSurfaceCommand)([this, TextureRenderTarget, &Buffer, CompressionQuality](FRHICommandListImmediate& RHICmdList)
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
		
	// Create empty texture
	FRHIResourceCreateInfo CreateInfo(TEXT("FreezeFrameTexture"));
	FTexture2DRHIRef DestTexture = GDynamicRHI->RHICreateTexture2D(Width, Height, EPixelFormat::PF_B8G8R8A8, 1, 1, TexCreate_RenderTargetable, ERHIAccess::Present, CreateInfo);

	// Copy freeze frame texture to empty texture
	CopyTextureBlock(Texture2DRHI, DestTexture, FIntPoint(0, 0), FIntPoint(1, 1));

	// Create the empty pixel surface to read the texture into
	TArray<FColor> Data;
	FIntRect Rect(0, 0, Width, Height);
	RHICmdList.ReadSurfaceData(DestTexture, Rect, Data, FReadSurfaceDataFlags());

	// Compress the surface data and set the byte data to the buffer
	CompressPixelsToBuffer(Data, Buffer, Width, Height, CompressionQuality);
});
}

void ULumafuseBufferQuadrantWorker::CompressPixelsToBuffer(TArray<FColor>& SurfaceData, TArray<uint8>& Buffer,
	int32 SizeX, int32 SizeY, int32 CompressionQuality)
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
