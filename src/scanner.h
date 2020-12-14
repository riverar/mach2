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
    class Scanner
    {
    public:
        enum class FeatureStage
        {
            Unknown = 0,
            AlwaysDisabled,
            DisabledByDefault,
            EnabledByDefault,
            AlwaysEnabled,
        };

        class Symbol
        {
        public:
            Symbol() {}
            Symbol(const std::wstring &path, GUID signature) { Path = path; Signature = signature; }
            bool operator==(const Symbol& other) const { return Path == other.Path; }
            std::wstring Path;
            GUID Signature{};
        };

        struct SymbolHasher
        {
            size_t operator()(const mach2::Scanner::Symbol& x) const
            {
                return std::hash<std::wstring>{}(x.Path);
            }
        };

        struct GuidHasher
        {
            size_t operator()(const GUID& x) const
            {
                RPC_STATUS status;
                return UuidHash(&const_cast<GUID&>(x), &status);
            }
        };

        struct GuidComparer
        {
            bool operator()(const GUID& left, const GUID& right) const
            {
                return IsEqualGUID(left, right);
            }
        };

        enum class SymbolHitType
        {
            Feature,
            FeatureGetter,
            Stage,
            Id,
        };

        class Feature
        {
        public:
            std::int64_t Id = 0;
            std::wstring Name;
            mach2::Scanner::FeatureStage Stage = mach2::Scanner::FeatureStage::Unknown;
            std::unordered_set<Symbol, SymbolHasher> Symbols;
            std::unordered_map<GUID, std::unordered_set<DWORD>, GuidHasher, GuidComparer> GetterRvasBySignature;
        };

        class Features
        {
        public:
            std::unordered_map<std::wstring, mach2::Scanner::Feature> FeaturesByName;
            std::unordered_map<mach2::Scanner::FeatureStage, std::map<std::wstring, mach2::Scanner::Feature*>> FeaturesByStage;
            std::unordered_map<GUID, std::unordered_set<mach2::Scanner::Feature*>, GuidHasher, GuidComparer> FeaturesByGetterSymbolSignature;
        };

        static const std::vector<FeatureStage> FeatureStages;
        typedef std::function<void(std::wstring const&)> Callback;

        Features GetFeaturesFromSymbolsAtPath(std::wstring const &path);
        void SetCallback(Callback const& callback);
        void GetMissingFeatureIdsFromImagesAtPath(mach2::Scanner::Features& features, std::wstring const& symbol_path, std::wstring const& image_path);
    private:
        Callback _callback;

        void ExecuteCallback(std::wstring const& path);
        std::wstring GetFeatureNameFromSymbolName(std::wstring const &symbolName);
        void GetFeaturesFromSymbolAtPath(std::wstring const &path, mach2::Scanner::Features &features);
        bool HasDuplicateFeatureWithId(std::int64_t featureId, std::wstring& featureName, mach2::Scanner::Features& features);
        std::wstring GetUniqueNameForDuplicateFeature(mach2::Scanner::Feature& feature, mach2::Scanner::Features& features);
        void InternalGetMissingFeatureIdsFromImagesAtPath(mach2::Scanner::Features& features, std::wstring const& image_path);
        void GetMissingFeatureIdsFromImageAtPath(std::wstring const& path, mach2::Scanner::Features& features);
        void InternalGetFeaturesFromSymbolsAtPath(std::wstring const &symbols_path, mach2::Scanner::Features &features);
        SymbolHitType GetSymbolHitTypeFromSymbolName(std::wstring const &symbolName);
    };
}