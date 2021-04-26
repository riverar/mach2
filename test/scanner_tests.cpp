#if !INLINE_TEST_METHOD_MARKUP
#define INLINE_TEST_METHOD_MARKUP
#endif

#include <WexTestClass.h>
#include "common.h"
#include "mach2.h"
#include "wnf.h"
#include "feature.h"
#include "feature_config.h"
#include "feature_manager.h"
#include "scanner.h"

class ScannerTests
{
	TEST_CLASS(ScannerTests)

	std::wstring _deploymentDirectory;
	TEST_CLASS_SETUP(ClassSetup)
	{
		WEX::Common::String deploymentDir;
		VERIFY_SUCCEEDED_RETURN(WEX::TestExecution::RuntimeParameters::TryGetValue(L"TestDeploymentDir", deploymentDir));
		_deploymentDirectory = deploymentDir;
		return true;
	}

	TEST_METHOD(GetFeaturesFromSymbolsAtPath_explorer_ContainsCorrectFeatureCount)
	{
		mach2::Scanner scanner{};
		auto features = scanner.GetFeaturesFromSymbolsAtPath(_deploymentDirectory + L"cases\\basic\\explorer.pdb");
		VERIFY_ARE_EQUAL(features.FeaturesByName.size(), 112);
	}

	TEST_METHOD(GetFeaturesFromSymbolsAtPath_explorer_FeatureIdInSymbolNotFound)
	{
		mach2::Scanner scanner{};
		auto features = scanner.GetFeaturesFromSymbolsAtPath(_deploymentDirectory + L"cases\\basic\\explorer.pdb");
		VERIFY_ARE_EQUAL(features.FeaturesByName[L"ScoobeEligibilityRedirection"].Id, 0);
	}

	TEST_METHOD(GetMissingFeatureIdsFromImagesAtPath_explorer_FeatureIdInImageFound)
	{
		mach2::Scanner scanner{};
		auto features = scanner.GetFeaturesFromSymbolsAtPath(_deploymentDirectory + L"cases\\basic\\explorer.pdb");
		scanner.GetMissingFeatureIdsFromImagesAtPath(features, _deploymentDirectory + L"cases\\basic\\explorer.pdb", _deploymentDirectory + L"cases\\basic");
		VERIFY_ARE_EQUAL(features.FeaturesByName[L"ScoobeEligibilityRedirection"].Id, 27336672);
	}

	TEST_METHOD(GetMissingFeatureIdsFromImagesAtPath_wux_FeatureCorrectlyStaged)
	{
		mach2::Scanner scanner{};
		auto features = scanner.GetFeaturesFromSymbolsAtPath(_deploymentDirectory + L"cases\\staging\\windows.ui.xaml.pdb");
		VERIFY_ARE_EQUAL(features.FeaturesByName[L"GraphicsCardInAbout"].Stage, mach2::Scanner::FeatureStage::DisabledByDefault);
	}

	TEST_METHOD(GetFeaturesFromSymbolsAtPath_authbroker_ContainsCorrectFeatureCount)
	{
		mach2::Scanner scanner{};
		auto features = scanner.GetFeaturesFromSymbolsAtPath(_deploymentDirectory + L"cases\\staging\\authbroker.pdb");
		VERIFY_ARE_EQUAL(features.FeaturesByName.size(), 25);
	}

	TEST_METHOD(GetFeaturesFromSymbolsAtPath_authbroker_ContainsCorrectAlwaysDisabledFeatureCount)
	{
		mach2::Scanner scanner{};
		auto features = scanner.GetFeaturesFromSymbolsAtPath(_deploymentDirectory + L"cases\\staging\\authbroker.pdb");
		VERIFY_ARE_EQUAL(features.FeaturesByStage[mach2::Scanner::FeatureStage::AlwaysDisabled].size(), 4);
	}

	TEST_METHOD(GetFeaturesFromSymbolsAtPath_authbroker_ContainsCorrectAlwaysDisabledFeatures)
	{
		mach2::Scanner scanner{};
		auto features = scanner.GetFeaturesFromSymbolsAtPath(_deploymentDirectory + L"cases\\staging\\authbroker.pdb");
		auto disabledFeatures = features.FeaturesByStage[mach2::Scanner::FeatureStage::AlwaysDisabled];
		
		VERIFY_ARE_NOT_EQUAL(disabledFeatures.find(L"AlternateHomeExperience"), disabledFeatures.end());
		VERIFY_ARE_NOT_EQUAL(disabledFeatures.find(L"AuthBroker_OnMobile"), disabledFeatures.end());
		VERIFY_ARE_NOT_EQUAL(disabledFeatures.find(L"HolographicCoreShellTLAVMOnOasis"), disabledFeatures.end());
		VERIFY_ARE_NOT_EQUAL(disabledFeatures.find(L"OASISAppFrameCustomization"), disabledFeatures.end());
	}

	TEST_METHOD(GetFeaturesFromSymbolsAtPath_authbroker_FeaturePromotedToHighestDiscoveredStage)
	{
		mach2::Scanner scanner{};
		auto features = scanner.GetFeaturesFromSymbolsAtPath(_deploymentDirectory + L"cases\\staging\\authbroker.pdb");
		auto alwaysEnabledFeatures = features.FeaturesByStage[mach2::Scanner::FeatureStage::AlwaysEnabled];
		auto enabledByDefaultFeatures = features.FeaturesByStage[mach2::Scanner::FeatureStage::EnabledByDefault];
		VERIFY_ARE_NOT_EQUAL(alwaysEnabledFeatures.find(L"FastPoseQuery"), alwaysEnabledFeatures.end());
		VERIFY_ARE_EQUAL(enabledByDefaultFeatures.find(L"FastPoseQuery"), enabledByDefaultFeatures.end());
	}

	TEST_METHOD(GetFeaturesFromSymbolsAtPath_dxgkrnl_ContainsCorrectFeatureCount)
	{
		mach2::Scanner scanner{};
		auto features = scanner.GetFeaturesFromSymbolsAtPath(_deploymentDirectory + L"cases\\basic\\dxgkrnl.pdb");
		VERIFY_ARE_EQUAL(features.FeaturesByName.size(), 58);
	}
};