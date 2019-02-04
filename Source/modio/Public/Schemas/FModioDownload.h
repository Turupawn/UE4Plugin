// Copyright 2019 modio. All Rights Reserved.
// Released under MIT.

#pragma once

#include "ModioHWrapper.h"
#include "FModioDownload.generated.h"

USTRUCT(BlueprintType)
struct FModioDownload
{
  GENERATED_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "mod.io")
  int32 DateExpires;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "mod.io")
  FString BinaryUrl;
};

extern void InitializeDownload(FModioDownload &download, const modio::Download &modio_download);
extern void InitializeDownloadC(FModioDownload &download, const ModioDownload &modio_download);