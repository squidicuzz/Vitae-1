// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2017 The VITAE developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

// clang-format off
#include "net.h"
#include "fundamentalnodeconfig.h"
#include "util.h"
#include "ui_interface.h"
#include <base58.h>
// clang-format on

CFundamentalnodeConfig fundamentalnodeConfig;

void CFundamentalnodeConfig::add(std::string alias, std::string ip, std::string privKey, std::string txHash, std::string outputIndex)
{
    CFundamentalnodeEntry cme(alias, ip, privKey, txHash, outputIndex);
    entries.push_back(cme);
}

bool CFundamentalnodeConfig::read(std::string& strErr)
{
    int linenumber = 1;
    boost::filesystem::path pathFundamentalnodeConfigFile = GetFundamentalnodeConfigFile();
    boost::filesystem::ifstream streamConfig(pathFundamentalnodeConfigFile);

    if (!streamConfig.good()) {
        FILE* configFile = fopen(pathFundamentalnodeConfigFile.string().c_str(), "a");
        if (configFile != NULL) {
            std::string strHeader = "# Fundamentalnode config file\n"
                                    "# Format: alias IP:port fundamentalnodeprivkey collateral_output_txid collateral_output_index\n"
                                    "# Example: mn1 127.0.0.2:8765 93HaYBVUCYjEMeeH1Y4sBGLALQZE1Yc1K64xiqgX37tGBDQL8Xg 2bcd3c84c84f87eaa86e4e56834c92927a07f9e18718810b92e0d0324456a67c 0\n";
            fwrite(strHeader.c_str(), std::strlen(strHeader.c_str()), 1, configFile);
            fclose(configFile);
        }
        return true; // Nothing to read, so just return
    }

    for (std::string line; std::getline(streamConfig, line); linenumber++) {
        if (line.empty()) continue;

        std::istringstream iss(line);
        std::string comment, alias, ip, privKey, txHash, outputIndex;

        if (iss >> comment) {
            if (comment.at(0) == '#') continue;
            iss.str(line);
            iss.clear();
        }

        if (!(iss >> alias >> ip >> privKey >> txHash >> outputIndex)) {
            iss.str(line);
            iss.clear();
            if (!(iss >> alias >> ip >> privKey >> txHash >> outputIndex)) {
                strErr = _("Could not parse fundamentalnode.conf") + "\n" +
                         strprintf(_("Line: %d"), linenumber) + "\n\"" + line + "\"";
                streamConfig.close();
                return false;
            }
        }

        if (Params().NetworkID() == CBaseChainParams::MAIN) {
            if (CService(ip).GetPort() != 8765) {
                strErr = _("Invalid port detected in fundamentalnode.conf") + "\n" +
                         strprintf(_("Line: %d"), linenumber) + "\n\"" + line + "\"" + "\n" +
                         _("(must be 8765 for mainnet)");
                streamConfig.close();
                return false;
            }
        } else if (CService(ip).GetPort() == 8765) {
            strErr = _("Invalid port detected in fundamentalnode.conf") + "\n" +
                     strprintf(_("Line: %d"), linenumber) + "\n\"" + line + "\"" + "\n" +
                     _("(8765 could be used only on mainnet)");
            streamConfig.close();
            return false;
        }


        add(alias, ip, privKey, txHash, outputIndex);
    }

    streamConfig.close();
    return true;
}

bool CFundamentalnodeConfig::CFundamentalnodeEntry::castOutputIndex(int &n)
{
    try {
        n = std::stoi(outputIndex);
    } catch (const std::exception e) {
        LogPrintf("%s: %s on getOutputIndex\n", __func__, e.what());
        return false;
    }

    return true;
}
