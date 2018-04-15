#pragma once

void DisplaySubcommand();
void ScanSubcommand(const std::string& symbols_path, const std::string& output_path, bool omit_symbol_hits);
void SetSubcommand(unsigned int feature_id, unsigned int service_state, unsigned int variant, bool add_to_machine_store);
