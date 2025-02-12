/*
** Copyright 2007-2024, RTE (https://www.rte-france.com)
** See AUTHORS.txt
** SPDX-License-Identifier: MPL-2.0
** This file is part of Antares-Simulator,
** Adequacy and Performance assessment for interconnected energy networks.
**
** Antares_Simulator is free software: you can redistribute it and/or modify
** it under the terms of the Mozilla Public Licence 2.0 as published by
** the Mozilla Foundation, either version 2 of the License, or
** (at your option) any later version.
**
** Antares_Simulator is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** Mozilla Public Licence 2.0 for more details.
**
** You should have received a copy of the Mozilla Public Licence 2.0
** along with Antares_Simulator. If not, see <https://opensource.org/license/mpl-2-0/>.
*/
#include <antares/inifile/inifile.h>
#include "antares/config/config.h"

#include "atsp.h"

using namespace Yuni;

#define SEP Yuni::IO::Separator

namespace Antares
{
bool ATSP::loadFromINIFile(const String& filename)
{
    pArea.clear();
    pStudyFolder.clear();
    pTemp.clear();
    pTimeseriesCount = 0;
    pMHeight = 0;
    pTimeseries = 0;
    pUseUpperBound = false;
    pUseLowerBound = false;
    pUpperBound = 100.;
    pLowerBound = 0.;
    pUpperBound80percent = 0.;
    pLimitMemory = 200 * 1024 * 1024;
    pAutoClean = false;

    IniFile ini;
    if (ini.open(filename))
    {
        logs.info() << "Reading " << filename;

        IniFile::Section* section;
        CString<50, false> key;
        CString<50, false> value;

        for (section = ini.firstSection; section; section = section->next)
        {
            if (section->name == ".general")
            {
                IniFile::Property* p = section->firstProperty;
                for (; p; p = p->next)
                {
                    key = p->key;
                    key.toLower();

                    if (key == "study")
                    {
                        pStudyFolder = p->value;
                        continue;
                    }
                    if (key == "temporary")
                    {
                        pTemp = p->value;
                        continue;
                    }

                    value = p->value;
                    value.trim(" \t\r\n");
                    value.toLower();

                    if (key == "target")
                    {
                        if (value == "load")
                        {
                            pTimeseries = Data::timeSeriesLoad;
                        }
                        else if (value == "solar")
                        {
                            pTimeseries = Data::timeSeriesSolar;
                        }
                        else if (value == "wind")
                        {
                            pTimeseries = Data::timeSeriesWind;
                        }
                        continue;
                    }
                    if (key == "width")
                    {
                        if (not value.to(pTimeseriesCount))
                        {
                            logs.error() << "impossible to read the number of timeseries";
                        }
                        continue;
                    }
                    if (key == "height")
                    {
                        if (not value.to(pMHeight))
                        {
                            logs.error() << "impossible to read the height";
                        }
                        continue;
                    }
                    if (key == "medium-term-autocorrelation")
                    {
                        pMediumTermAutoCorrAdjustment = value.to<double>() / 100.;
                        continue;
                    }
                    if (key == "short-term-autocorrelation")
                    {
                        pShortTermAutoCorrAdjustment = value.to<double>() / 100.;
                        continue;
                    }
                    if (key == "trimming")
                    {
                        pRoundOff = value.to<double>() / 100.;
                        continue;
                    }
                    if (key == "upperbound-enable")
                    {
                        pUseUpperBound = value.to<bool>();
                        continue;
                    }
                    if (key == "lowerbound-enable")
                    {
                        pUseLowerBound = value.to<bool>();
                        continue;
                    }
                    if (key == "upperbound-value")
                    {
                        pUpperBound = value.to<double>();
                        pUpperBound80percent = pUpperBound * 0.8;
                        continue;
                    }
                    if (key == "lowerbound-value")
                    {
                        pLowerBound = value.to<double>();
                        continue;
                    }
                    if (key == "memory-cache")
                    {
                        pLimitMemory = value.to<size_t>();
                        if (pLimitMemory > 16384)
                        {
                            logs.warning()
                              << "The limit of the memory cache size has been shrinked to 16Go";
                            pLimitMemory = 16384u; // splitted into 2 parts to avoid constant out of
                                                   // range on Visual Studio
                            pLimitMemory *= 1024u * 1024u;
                        }
                        else
                        {
                            pLimitMemory *= 1024u * 1024u;
                        }
                        continue;
                    }
                    if (key == "clean")
                    {
                        pAutoClean = value.to<bool>();
                        continue;
                    }
                }
            }
            else
            {
                AreaInfo info;
                info.name = section->name;
                info.name.toLower();
                info.enabled = true;
                info.distribution = Data::XCast::dtBeta;

                IniFile::Property* p = section->firstProperty;
                for (; p; p = p->next)
                {
                    key = p->key;
                    key.toLower();

                    if (key == "file")
                    {
                        info.filename = p->value;
                        continue;
                    }
                    if (key == "data")
                    {
                        key = p->value;
                        key.toLower();
                        info.rawData = (key == "raw");
                        continue;
                    }
                    if (key == "distribution")
                    {
                        info.distribution = Data::XCast::StringToDistribution(p->value);
                        if (info.distribution == Data::XCast::dtNone)
                        {
                            logs.error() << "invalid distribution for " << section->name
                                         << " ('beta' will be used)";
                            info.distribution = Data::XCast::dtBeta;
                        }
                        continue;
                    }
                }

                pArea.push_back(std::move(info));
            }
        }

        if (pMHeight < 8760 or pMHeight > 9000)
        {
            logs.error() << "invalid height";
            return false;
        }
        if (pArea.empty())
        {
            logs.error() << "no area found.";
            return false;
        }

        if (!checkStudyVersion())
        {
            return false;
        }

        logs.info() << "Target study: " << pStudyFolder;

        NBZ = (uint)pArea.size();
        NBS = pTimeseriesCount;
        TDS = 1;
        RTZ = pRoundOff;
        code = 'W';
        switch (pTimeseries)
        {
        case Data::timeSeriesLoad:
            TDS = 3;
            code = 'L';
            tsName = "load";
            break;
        case Data::timeSeriesSolar:
            TDS = 2;
            code = 'S';
            tsName = "solar";
            break;
        case Data::timeSeriesWind:
            TDS = 1;
            code = 'W';
            tsName = "wind";
            break;
        default:
            TDS = -1;
            code = '_';
            tsName = '_';
            logs.error() << "invalid timeseries type. Expected load, solar, or wind";
            return false;
        }

        AUC = pShortTermAutoCorrAdjustment;
        AUM = pMediumTermAutoCorrAdjustment;

        SERIE_N.resize(pTimeseriesCount, 744);
        SERIE_P.resize(pTimeseriesCount, 744);
        SERIE_Q.resize(pTimeseriesCount, 744);

        folderPerArea.resize(pArea.size());
        moments_centr_net.resize(pArea.size());
        moments_centr_raw.resize(pArea.size());
        hidden_hours.resize(pArea.size());

        // Copying the INI file
        pStr.clear() << pTemp << SEP << tsName << SEP << "settings.ini";
        logs.info() << " Exporting settings to " << pStr;
        IO::File::Copy(filename, pStr);

        // Checking  0 < AUM < AUC < 1
        if (AUC <= 0. or AUC >= 1.)
        {
            logs.error()
              << "The short-term auto-correlation adjustment must be strictly between 0 and 1";
            return false;
        }
        if (AUM <= 0. or AUM >= 1.)
        {
            logs.error()
              << "The medium-term auto-correlation adjustment must be strictly between 0 and 1";
            return false;
        }
        if (AUC < AUM)
        {
            logs.error() << "The medium-term auto-correlation adjustment must be strictly less "
                            "than the short-term";
            return false;
        }

        return true;
    }
    return false;
}

bool ATSP::checkStudyVersion() const
{
    auto v = Data::StudyHeader::tryToFindTheVersion(pStudyFolder);
    if (v == Data::StudyVersion::unknown())
    {
        logs.error() << "Couldn't indentify the study version, is the folder a correct study ?";
        return false;
    }
    if (v > Data::StudyVersion::latest())
    {
        logs.error() << "The format of the study folder requires a more recent version of Antares";
        return false;
    }
    if (v < Data::StudyVersion::latest())
    {
        logs.error() << "The study folder must be upgraded from v" << v.toString() << " to v"
                     << Data::StudyVersion::latest().toString();
        return false;
    }
    return true;
}

} // namespace Antares
