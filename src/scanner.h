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

        enum class SymbolHitType
        {
            Feature,
            Stage,
            Id
        };

        class Feature
        {
        public:
            std::int32_t Id = 0;
            std::wstring Name;
            mach2::Scanner::FeatureStage Stage = mach2::Scanner::FeatureStage::Unknown;
            std::unordered_set<std::wstring> SymbolPaths;
        };

        class Features
        {
        public:
            std::unordered_map<std::wstring, mach2::Scanner::Feature> FeaturesByName;
            std::unordered_map<mach2::Scanner::FeatureStage, std::map<std::wstring, mach2::Scanner::Feature*>> FeaturesByStage;
        };

        static const std::vector<FeatureStage> FeatureStages;
        typedef std::function<void(std::wstring const &)> Callback;

        Features GetFeaturesFromSymbolsAtPath(std::wstring const &path);
        void SetCallback(Callback const &callback);
    private:
        std::function<void(std::wstring const &)> _callback;

        void ExecuteCallback(std::wstring const &pdb_path);
        std::wstring GetFeatureNameFromSymbolName(std::wstring const &symbolName);
        void GetFeaturesFromSymbolAtPath(std::wstring const &path, mach2::Scanner::Features &features);
        void InternalGetFeaturesFromSymbolsAtPath(std::wstring const &symbols_path, mach2::Scanner::Features &features);
        SymbolHitType GetSymbolHitTypeFromSymbolName(std::wstring const &symbolName);
    };
}