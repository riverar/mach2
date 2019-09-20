/*
    mach2 - feature control multi-tool
    Copyright (c) Rafael Rivera

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

namespace mach2
{
    typedef struct _WNF_STATE_NAME
    {
        ULONG Data[2];
    } WNF_STATE_NAME, *PWNF_STATE_NAME;

    typedef struct _WNF_TYPE_ID
    {
        GUID TypeId;
    } WNF_TYPE_ID, *PWNF_TYPE_ID;
    typedef const WNF_TYPE_ID *PCWNF_TYPE_ID;

    typedef ULONG WNF_CHANGE_STAMP, *PWNF_CHANGE_STAMP;

    extern "C"
        NTSTATUS
        NTAPI
        NtQueryWnfStateData(
            _In_ PWNF_STATE_NAME StateName,
            _In_opt_ PWNF_TYPE_ID TypeId,
            _In_opt_ const VOID* ExplicitScope,
            _Out_ PWNF_CHANGE_STAMP ChangeStamp,
            _Out_writes_bytes_to_opt_(*BufferSize, *BufferSize) PVOID Buffer,
            _Inout_ PULONG BufferSize);

    extern "C"
        NTSTATUS
        NTAPI
        NtUpdateWnfStateData(
            _In_ PWNF_STATE_NAME StateName,
            _In_reads_bytes_opt_(Length) const VOID* Buffer,
            _In_opt_ ULONG Length,
            _In_opt_ PCWNF_TYPE_ID TypeId,
            _In_opt_ const PVOID ExplicitScope,
            _In_ WNF_CHANGE_STAMP MatchingChangeStamp,
            _In_ ULONG CheckStamp);

    struct wil_details_StagingConfigHeaderProperties
    {
        unsigned __int32 ignoreServiceState : 1;
        unsigned __int32 ignoreUserState : 1;
        unsigned __int32 ignoreTestState : 1;
        unsigned __int32 ignoreVariants : 1;
        unsigned __int32 unused : 28;
    };

    struct wil_details_StagingConfigHeader
    {
        char version;
        char versionMinor;
        unsigned __int16 headerSizeBytes;
        unsigned __int16 featureCount;
        unsigned __int16 featureUsageTriggerCount;
        wil_details_StagingConfigHeaderProperties sessionProperties;
        wil_details_StagingConfigHeaderProperties properties;
    };

    enum wil_FeatureStore
    {
        wil_FeatureStore_Machine = 0x0,
        wil_FeatureStore_User = 0x1,
        wil_FeatureStore_All = 0x2
    };

    struct wil_details_StagingConfigFeature
    {
        unsigned int featureId;
        unsigned __int32 changedInSession : 1;
        unsigned __int32 unused1 : 7;
        unsigned __int32 serviceState : 2;
        unsigned __int32 userState : 2;
        unsigned __int32 testState : 2;
        unsigned __int32 unused2 : 2;
        unsigned __int32 unused3 : 8;
        unsigned __int32 variant : 6;
        unsigned __int32 payloadKind : 2;
        unsigned int payload;
    };

    struct wil_details_StagingConfigWnfStateName
    {
        unsigned int Data[2];
    };

    struct wil_details_StagingConfigUsageTrigger
    {
        unsigned int featureId;
        wil_details_StagingConfigWnfStateName trigger;
        unsigned __int16 serviceReportingKind;
        unsigned __int16 unused;
    };

    struct wil_details_StagingConfig
    {
        wil_FeatureStore store;
        int forUpdate;
        unsigned int readChangeStamp;
        char readVersion;
        int modified;
        wil_details_StagingConfigHeader *header;
        wil_details_StagingConfigFeature *features;
        wil_details_StagingConfigUsageTrigger *triggers;
        int changedInSession;
        void *buffer;
        unsigned int bufferSize;
        unsigned int bufferAlloc;
        int bufferOwned;
    };

    enum wil_details_StagingConfigFeatureFields
    {
        wil_details_StagingConfigFeatureFields_None = 0x0,
        wil_details_StagingConfigFeatureFields_ServiceState = 0x1,
        wil_details_StagingConfigFeatureFields_UserState = 0x2,
        wil_details_StagingConfigFeatureFields_TestState = 0x4,
        wil_details_StagingConfigFeatureFields_Variant = 0x8,
    };

    enum wil_FeatureEnabledState
    {
        Default = 0,
        Disabled = 1,
        Enabled = 2,
		HasVariantConfiguration = 64,
        HasNotification = 128
    };

    enum WnfStateName
    {
        UserFeatureStore,
        MachineFeatureStore,
        UserModifiedFeatureStore,
        MachineModifiedFeatureStore,
        FeatureUsageTracking01,
        FeatureUsageTracking02,
        FeatureUsageTracking03
    };

    enum wil_VariantPayloadType
    {
        wil_VariantPayloadType_None = 0x0,
        wil_VariantPayloadType_Fixed = 0x1
    };

    enum wil_FeatureStage
    {
        AlwaysDisabled = 0x0,
        DisabledByDefault,
        EnabledByDefault,
        AlwaysEnabled
    };

    struct wil_details_FeatureProperties
    {
        unsigned __int32 enabledState : 2;
        unsigned __int32 isVariant : 1;
        unsigned __int32 queuedForReporting : 1;
        unsigned __int32 hasNotificationState : 2;
        unsigned __int32 usageCount : 9;
        unsigned __int32 usageCountRepresentsPotential : 1;
        unsigned __int32 reportedDeviceUsage : 1;
        unsigned __int32 reportedDevicePotential : 1;
        unsigned __int32 reportedDeviceOpportunity : 1;
        unsigned __int32 reportedDevicePotentialOpportunity : 1;
        unsigned __int32 recordedDeviceUsage : 1;
        unsigned __int32 recordedDevicePotential : 1;
        unsigned __int32 recordedDeviceOpportunity : 1;
        unsigned __int32 recordedDevicePotentialOpportunity : 1;
        unsigned __int32 opportunityCount : 7;
        unsigned __int32 opportunityCountRepresentsPotential : 1;
    };

    struct wil_details_VariantProperties
    {
        unsigned __int32 enabledState : 2;
        unsigned __int32 isVariant : 1;
        unsigned __int32 queuedForReporting : 1;
        unsigned __int32 hasNotificationState : 2;
        unsigned __int32 recordedDeviceUsage : 1;
        unsigned __int32 variant : 6;
        unsigned __int32 unused : 19;
    };
}