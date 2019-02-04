// Copyright 2019 modio. All Rights Reserved.
// Released under MIT.

#include "FModioModule.h"
#if WITH_EDITOR
#include "ISettingsModule.h"
#include "ISettingsContainer.h"
#include "ISettingsSection.h"
#endif

#define LOCTEXT_NAMESPACE "FModioModule"

modio::Instance *modio_instance;
std::string current_user_username;

void FModioModule::StartupModule()
{
  const UModioSettings *Settings = GetDefault<UModioSettings>();

  FString game_directory = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
  std::string game_directory_str = std::string(TCHAR_TO_UTF8(*game_directory));

  u32 environment;
  if (Settings->IsLiveEnvironment)
    environment = MODIO_ENVIRONMENT_LIVE;
  else
    environment = MODIO_ENVIRONMENT_TEST;
  
  if (Settings->RootDirectory != "")
    game_directory_str += std::string(TCHAR_TO_UTF8(*Settings->RootDirectory));

  u32 game_id = Settings->GameId;
  std::string api_key = std::string(TCHAR_TO_UTF8(*(Settings->ApiKey)));

  modio_instance = new modio::Instance(environment, game_id, api_key, game_directory_str);

  modio_instance->setDownloadListener([&](u32 response_code, u32 mod_id) {
    UModioComponent::OnModDownloadDelegate.Broadcast((int32)response_code);
  });

  modio_instance->setUploadListener([&](u32 response_code, u32 mod_id) {
    UModioComponent::OnModUploadDelegate.Broadcast((int32)response_code);
  });

  current_user_username = "";

  modio_instance->getAuthenticatedUser([&](const modio::Response &response, const modio::User &user) {
    current_user_username = user.username;
  });

  // Need GIsEdtor check as this might run when running the game but not with the editor
  if( GIsEditor )
  {
    RegisterSettings();
  }
}

void FModioModule::ShutdownModule()
{
  // Need GIsEdtor check as this might run when running the game but not with the editor	
  if ( UObjectInitialized() && GIsEditor )
  {
    UnregisterSettings();
  }
  delete modio_instance;
}

bool FModioModule::SupportsDynamicReloading()
{
  return true;
}

bool FModioModule::HandleSettingsSaved()
{
  UModioSettings *Settings = GetMutableDefault<UModioSettings>();
  bool ResaveSettings = false;

  if (ResaveSettings)
  {
    Settings->SaveConfig();
  }

  return true;
}

// @todo: Move Register/Unregister Settings to editor module as they use editor only ISettingsModule
void FModioModule::RegisterSettings()
{
  #if WITH_EDITOR
  if (ISettingsModule *SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
  {
    ISettingsContainerPtr SettingsContainer = SettingsModule->GetContainer("Project");

    SettingsContainer->DescribeCategory("mod.io settings",
                                        LOCTEXT("RuntimeWDCategoryName", "mod.io settings"),
                                        LOCTEXT("RuntimeWDCategoryDescription", "mod.io configuration settings"));

    ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings("Project", "mod.io settings", "General",
                                                                           LOCTEXT("RuntimeGeneralSettingsName", "General"),
                                                                           LOCTEXT("RuntimeGeneralSettingsDescription", "Base configuration for our game module"),
                                                                           GetMutableDefault<UModioSettings>());

    if (SettingsSection.IsValid())
    {
      SettingsSection->OnModified().BindRaw(this, &FModioModule::HandleSettingsSaved);
    }
  }
  #endif
}

void FModioModule::UnregisterSettings()
{
  #if WITH_EDITOR
  if (ISettingsModule *SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
  {
    SettingsModule->UnregisterSettings("Project", "mod.io Settings", "General");
  }
  #endif
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FModioModule, modio)