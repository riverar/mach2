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

#include "common.h"
#include "mach2.h"
#include "wnf.h"
#include "feature.h"
#include "feature_config.h"
#include "feature_manager.h"
#include "scanner.h"
#include "CLI/CLI.hpp"

// CLI11 doesn't support wide-character sets yet
// https://github.com/CLIUtils/CLI11/issues/14
//
int main(int argc, char *argv[], char *)
{
    std::wcout
        << L"mach2 0.8 - Feature Control Multi-tool" << std::endl
        << L"Copyright (c) Rafael Rivera" << std::endl
        << std::endl
        << L"This program comes with ABSOLUTELY NO WARRANTY." << std::endl
        << L"This is free software, and you are welcome to redistribute it under certain conditions." << std::endl
        << std::endl;

    try
    {
        CCoInitialize com_init;

        CLI::App app;
        app.require_subcommand(1, 1);

        std::string symbol_path;
        std::string output_path;
        std::string image_path;
        auto scan = app.add_subcommand("scan", "Scan the provided path for symbols (*.pdb) and print any features found")
            ->group("Feature Discovery");
        scan->add_option("path", symbol_path, "Path to symbols")
            ->required()
            ->check(CLI::ExistingDirectory);
        scan->add_option("-i,--image-path", image_path, "Path to executable images")
            ->check(CLI::ExistingDirectory);
        scan->add_option("-o,--output", output_path, "Output results to file at <path>");
        scan->add_flag("-s,--strip-paths", "Strip symbol paths from output (makes diffing easier)")
            ->needs("-o");
        scan->add_flag("-u,--use-capstone")
            ->needs("-i");
        scan->callback([&]() { ScanSubcommand(symbol_path, output_path, image_path, scan->count("-s") > 0, scan->count("--use-capstone") > 0); });

        app.add_subcommand("display", "Display configured features on the local machine")
            ->group("Feature Management")
            ->callback([&]() { DisplaySubcommand(); });

        std::uint32_t feature_ids;
        std::uint32_t variants;
        auto enable = app.add_subcommand("enable", "Enable feature on the local machine")
            ->group("Feature Management");
        enable->add_option("featureId", feature_ids)
            ->required();
        enable->add_option("-v,--variant", variants, "Set feature variant(s)")
            ->default_val("0");
        enable->callback([&]() { SetSubcommand(feature_ids, mach2::wil_FeatureStage::EnabledByDefault, variants, true); });

        auto disable = app.add_subcommand("disable", "Disable feature on the local machine")
            ->group("Feature Management");
        disable->add_option("featureId", feature_ids)
            ->required();
        disable->callback([&]() { SetSubcommand(feature_ids, mach2::wil_FeatureStage::DisabledByDefault, variants, true); });

        auto revert = app.add_subcommand("revert", "Revert feature to default configuration on the local machine")
            ->group("Feature Management");
        revert->add_option("featureId", feature_ids)
            ->required();
        revert->callback([&]() { SetSubcommand(feature_ids, mach2::wil_FeatureStage::AlwaysDisabled /* ignored */, variants, false); });

        CLI11_PARSE(app, argc, argv);
    }
    catch (std::system_error & e)
    {
        std::cout << e.what() << ", " << std::hex << e.code() << std::endl;
    }
    catch (std::exception& e)
    {
        std::cout << e.what() << std::endl;
        return 1;
    }

    return 0;
}