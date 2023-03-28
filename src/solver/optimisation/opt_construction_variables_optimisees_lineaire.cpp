/*
** Copyright 2007-2018 RTE
** Authors: Antares_Simulator Team
**
** This file is part of Antares_Simulator.
**
** Antares_Simulator is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** There are special exceptions to the terms and conditions of the
** license as they are applied to this software. View the full text of
** the exceptions in file COPYING.txt in the directory of this software
** distribution
**
** Antares_Simulator is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Antares_Simulator. If not, see <http://www.gnu.org/licenses/>.
**
** SPDX-License-Identifier: licenceRef-GPL3_WITH_RTE-Exceptions
*/
#include "opt_structure_probleme_a_resoudre.h"

#include "../simulation/simulation.h"
#include "../simulation/sim_structure_donnees.h"
#include "../simulation/sim_extern_variables_globales.h"

#include "opt_fonctions.h"
#include <math.h>

#include "spx_constantes_externes.h"

void OPT_ConstruireLaListeDesVariablesOptimiseesDuProblemeLineaire(PROBLEME_HEBDO* ProblemeHebdo)
{
    int Interco;
    int Pays;
    int Pdt;
    int Palier;
    int NombreDeVariables;
    int Index;
    char Simulation;
    int NombreDePasDeTempsPourUneOptimisation;
    char ContrainteDeReserveJMoins1ParZone;
    CORRESPONDANCES_DES_VARIABLES* CorrespondanceVarNativesVarOptim;
    PALIERS_THERMIQUES* PaliersThermiquesDuPays;
    COUTS_DE_TRANSPORT* CoutDeTransport;
    PROBLEME_ANTARES_A_RESOUDRE* ProblemeAResoudre;

    ProblemeAResoudre = ProblemeHebdo->ProblemeAResoudre;

    NombreDePasDeTempsPourUneOptimisation = ProblemeHebdo->NombreDePasDeTempsPourUneOptimisation;
    ContrainteDeReserveJMoins1ParZone = ProblemeHebdo->ContrainteDeReserveJMoins1ParZone;

    NombreDeVariables = 0;

    for (Pdt = 0; Pdt < NombreDePasDeTempsPourUneOptimisation; Pdt++)
    {
        CorrespondanceVarNativesVarOptim = ProblemeHebdo->CorrespondanceVarNativesVarOptim[Pdt];

        for (Interco = 0; Interco < ProblemeHebdo->NombreDInterconnexions; Interco++)
        {
            CorrespondanceVarNativesVarOptim->NumeroDeVariableDeLInterconnexion[Interco]
              = NombreDeVariables;
            ProblemeAResoudre->TypeDeVariable[NombreDeVariables] = VARIABLE_BORNEE_DES_DEUX_COTES;
            NombreDeVariables++;

            CoutDeTransport = ProblemeHebdo->CoutDeTransport[Interco];
            if (CoutDeTransport->IntercoGereeAvecDesCouts == OUI_ANTARES)
            {
                CorrespondanceVarNativesVarOptim
                  ->NumeroDeVariableCoutOrigineVersExtremiteDeLInterconnexion[Interco]
                  = NombreDeVariables;
                ProblemeAResoudre->TypeDeVariable[NombreDeVariables]
                  = VARIABLE_BORNEE_DES_DEUX_COTES;
                NombreDeVariables++;
                CorrespondanceVarNativesVarOptim
                  ->NumeroDeVariableCoutExtremiteVersOrigineDeLInterconnexion[Interco]
                  = NombreDeVariables;
                ProblemeAResoudre->TypeDeVariable[NombreDeVariables]
                  = VARIABLE_BORNEE_DES_DEUX_COTES;
                NombreDeVariables++;
            }
        }

        for (Pays = 0; Pays < ProblemeHebdo->NombreDePays; Pays++)
        {
            PaliersThermiquesDuPays = ProblemeHebdo->PaliersThermiquesDuPays[Pays];
            for (Index = 0; Index < PaliersThermiquesDuPays->NombreDePaliersThermiques; Index++)
            {
                Palier
                  = PaliersThermiquesDuPays->NumeroDuPalierDansLEnsembleDesPaliersThermiques[Index];
                CorrespondanceVarNativesVarOptim->NumeroDeVariableDuPalierThermique[Palier]
                  = NombreDeVariables;
                ProblemeAResoudre->TypeDeVariable[NombreDeVariables]
                  = VARIABLE_BORNEE_DES_DEUX_COTES;
                NombreDeVariables++;
            }

            CorrespondanceVarNativesVarOptim->NumeroDeVariableDefaillancePositive[Pays]
              = NombreDeVariables;

            ProblemeAResoudre->TypeDeVariable[NombreDeVariables] = VARIABLE_BORNEE_DES_DEUX_COTES;
            NombreDeVariables++;

            CorrespondanceVarNativesVarOptim->NumeroDeVariableDefaillanceNegative[Pays]
              = NombreDeVariables;

            ProblemeAResoudre->TypeDeVariable[NombreDeVariables] = VARIABLE_BORNEE_INFERIEUREMENT;
            NombreDeVariables++;

            if (ContrainteDeReserveJMoins1ParZone == OUI_ANTARES)
            {
                CorrespondanceVarNativesVarOptim->NumeroDeVariableDefaillanceEnReserve[Pays]
                  = NombreDeVariables;
                ProblemeAResoudre->TypeDeVariable[NombreDeVariables]
                  = VARIABLE_BORNEE_INFERIEUREMENT;
                NombreDeVariables++;
            }
            else
                CorrespondanceVarNativesVarOptim->NumeroDeVariableDefaillanceEnReserve[Pays] = -1;
        }

        for (Pays = 0; Pays < ProblemeHebdo->NombreDePays; Pays++)
        {
            if (ProblemeHebdo->CaracteristiquesHydrauliques[Pays]->PresenceDHydrauliqueModulable
                == OUI_ANTARES)
            {
                CorrespondanceVarNativesVarOptim->NumeroDeVariablesDeLaProdHyd[Pays]
                  = NombreDeVariables;
                ProblemeAResoudre->TypeDeVariable[NombreDeVariables]
                  = VARIABLE_BORNEE_DES_DEUX_COTES;
                NombreDeVariables++;
            }
            else
                CorrespondanceVarNativesVarOptim->NumeroDeVariablesDeLaProdHyd[Pays] = -1;

            CorrespondanceVarNativesVarOptim->NumeroDeVariablesVariationHydALaBaisse[Pays] = -1;
            CorrespondanceVarNativesVarOptim->NumeroDeVariablesVariationHydALaHausse[Pays] = -1;
            if (ProblemeHebdo->TypeDeLissageHydraulique
                == LISSAGE_HYDRAULIQUE_SUR_SOMME_DES_VARIATIONS)
            {
                if (ProblemeHebdo->CaracteristiquesHydrauliques[Pays]->PresenceDHydrauliqueModulable
                    == OUI_ANTARES)
                {
                    CorrespondanceVarNativesVarOptim->NumeroDeVariablesVariationHydALaBaisse[Pays]
                      = NombreDeVariables;
                    ProblemeAResoudre->TypeDeVariable[NombreDeVariables]
                      = VARIABLE_BORNEE_INFERIEUREMENT;
                    NombreDeVariables++;
                    CorrespondanceVarNativesVarOptim->NumeroDeVariablesVariationHydALaHausse[Pays]
                      = NombreDeVariables;
                    ProblemeAResoudre->TypeDeVariable[NombreDeVariables]
                      = VARIABLE_BORNEE_INFERIEUREMENT;
                    NombreDeVariables++;
                }
            }
            else if (ProblemeHebdo->TypeDeLissageHydraulique
                     == LISSAGE_HYDRAULIQUE_SUR_VARIATION_MAX)
            {
                if (ProblemeHebdo->CaracteristiquesHydrauliques[Pays]->PresenceDHydrauliqueModulable
                    == OUI_ANTARES)
                {
                    if (Pdt == 0)
                    {
                        CorrespondanceVarNativesVarOptim
                          ->NumeroDeVariablesVariationHydALaBaisse[Pays]
                          = NombreDeVariables;

                        ProblemeAResoudre->TypeDeVariable[NombreDeVariables]
                          = VARIABLE_BORNEE_DES_DEUX_COTES;
                        NombreDeVariables++;
                        CorrespondanceVarNativesVarOptim
                          ->NumeroDeVariablesVariationHydALaHausse[Pays]
                          = NombreDeVariables;

                        ProblemeAResoudre->TypeDeVariable[NombreDeVariables]
                          = VARIABLE_BORNEE_DES_DEUX_COTES;
                        NombreDeVariables++;
                    }
                }
            }

            if (ProblemeHebdo->CaracteristiquesHydrauliques[Pays]->PresenceDePompageModulable
                == OUI_ANTARES)
            {
                CorrespondanceVarNativesVarOptim->NumeroDeVariablesDePompage[Pays]
                  = NombreDeVariables;
                ProblemeAResoudre->TypeDeVariable[NombreDeVariables]
                  = VARIABLE_BORNEE_DES_DEUX_COTES;
                NombreDeVariables++;
            }
            else
                CorrespondanceVarNativesVarOptim->NumeroDeVariablesDePompage[Pays] = -1;

            if (ProblemeHebdo->CaracteristiquesHydrauliques[Pays]->SuiviNiveauHoraire
                == OUI_ANTARES)
            {
                CorrespondanceVarNativesVarOptim->NumeroDeVariablesDeNiveau[Pays]
                  = NombreDeVariables;
                ProblemeAResoudre->TypeDeVariable[NombreDeVariables]
                  = VARIABLE_BORNEE_DES_DEUX_COTES;
                NombreDeVariables++;
                CorrespondanceVarNativesVarOptim->NumeroDeVariablesDeDebordement[Pays]
                  = NombreDeVariables;
                ProblemeAResoudre->TypeDeVariable[NombreDeVariables]
                  = VARIABLE_BORNEE_DES_DEUX_COTES;
                NombreDeVariables++;
            }
            else
            {
                CorrespondanceVarNativesVarOptim->NumeroDeVariablesDeNiveau[Pays] = -1;
                CorrespondanceVarNativesVarOptim->NumeroDeVariablesDeDebordement[Pays] = -1;
            }
        }

        // ===== HYDRO-CLUSTERS-START ===== //
        for (Pays = 0; Pays < ProblemeHebdo->NombreDePays; Pays++)
        {
            PALIERS_HYDROCLUSTERS& PaliersHydroclusterDuPays
              = ProblemeHebdo->PaliersHydroclusterDuPays[Pays];
            uint areaClusterCount = PaliersHydroclusterDuPays.areaClusterCount;
            for (int cluster = 0; cluster < areaClusterCount; cluster++)
            {
                auto& clusterHydroData = PaliersHydroclusterDuPays.hydroClusterMap.at(cluster);
                int clusterIndex = PaliersHydroclusterDuPays.clusterIndexTotalCount[cluster];

                if (clusterHydroData.PresenceDHydrauliqueModulable == OUI_ANTARES)
                {
                    CorrespondanceVarNativesVarOptim->NumberOfVariablesProdHydClu[clusterIndex]
                      = NombreDeVariables;
                    ProblemeAResoudre->TypeDeVariable[NombreDeVariables]
                      = VARIABLE_BORNEE_DES_DEUX_COTES;
                    NombreDeVariables++;
                }
                else
                    CorrespondanceVarNativesVarOptim->NumberOfVariablesProdHydClu[Pays] = -1;

                CorrespondanceVarNativesVarOptim
                  ->NumeroDeVariablesVariationHydALaBaisseClu[clusterIndex]
                  = -1;
                CorrespondanceVarNativesVarOptim
                  ->NumeroDeVariablesVariationHydALaHausseClu[clusterIndex]
                  = -1;
                if (ProblemeHebdo->TypeDeLissageHydraulique
                    == LISSAGE_HYDRAULIQUE_SUR_SOMME_DES_VARIATIONS) // HYDRAULIC SMOOTHING ON SUM
                                                                     // OF VARIATIONS
                {
                    if (clusterHydroData.PresenceDHydrauliqueModulable == OUI_ANTARES)
                    {
                        CorrespondanceVarNativesVarOptim
                          ->NumeroDeVariablesVariationHydALaBaisseClu[clusterIndex]
                          = NombreDeVariables;
                        ProblemeAResoudre->TypeDeVariable[NombreDeVariables]
                          = VARIABLE_BORNEE_INFERIEUREMENT;
                        NombreDeVariables++;
                        CorrespondanceVarNativesVarOptim
                          ->NumeroDeVariablesVariationHydALaHausseClu[clusterIndex]
                          = NombreDeVariables;
                        ProblemeAResoudre->TypeDeVariable[NombreDeVariables]
                          = VARIABLE_BORNEE_INFERIEUREMENT; // LOWER_BOUNDED VARIABLE
                        NombreDeVariables++;
                    }
                }
                else if (ProblemeHebdo->TypeDeLissageHydraulique
                         == LISSAGE_HYDRAULIQUE_SUR_VARIATION_MAX) // HYDRAULIC SMOOTHING ON MAX
                                                                   // VARIATION
                {
                    if (clusterHydroData.PresenceDHydrauliqueModulable == OUI_ANTARES)
                    {
                        if (Pdt == 0)
                        {
                            CorrespondanceVarNativesVarOptim
                              ->NumeroDeVariablesVariationHydALaBaisseClu[clusterIndex]
                              = NombreDeVariables;

                            ProblemeAResoudre->TypeDeVariable[NombreDeVariables]
                              = VARIABLE_BORNEE_DES_DEUX_COTES; // VARIABLE BOUNDED on BOTH SIDES
                            NombreDeVariables++;
                            CorrespondanceVarNativesVarOptim
                              ->NumeroDeVariablesVariationHydALaHausseClu[clusterIndex]
                              = NombreDeVariables;

                            ProblemeAResoudre->TypeDeVariable[NombreDeVariables]
                              = VARIABLE_BORNEE_DES_DEUX_COTES;
                            NombreDeVariables++;
                        }
                    }
                }

                if (clusterHydroData.PresenceDePompageModulable == OUI_ANTARES)
                {
                    CorrespondanceVarNativesVarOptim->NumberOfVariablesPumpHydClu[clusterIndex]
                      = NombreDeVariables;
                    ProblemeAResoudre->TypeDeVariable[NombreDeVariables]
                      = VARIABLE_BORNEE_DES_DEUX_COTES;
                    NombreDeVariables++;
                }
                else
                    CorrespondanceVarNativesVarOptim->NumberOfVariablesPumpHydClu[clusterIndex]
                      = -1;

                if (clusterHydroData.SuiviNiveauHoraire == OUI_ANTARES)
                {
                    CorrespondanceVarNativesVarOptim->NumberOfVariablesLevelClu[clusterIndex]
                      = NombreDeVariables;
                    ProblemeAResoudre->TypeDeVariable[NombreDeVariables]
                      = VARIABLE_BORNEE_DES_DEUX_COTES;
                    NombreDeVariables++;
                    CorrespondanceVarNativesVarOptim->NumberOfVariablesOverflowClu[clusterIndex]
                      = NombreDeVariables;
                    ProblemeAResoudre->TypeDeVariable[NombreDeVariables]
                      = VARIABLE_BORNEE_DES_DEUX_COTES;
                    NombreDeVariables++;
                }
                else
                {
                    CorrespondanceVarNativesVarOptim->NumberOfVariablesLevelClu[clusterIndex] = -1;
                    CorrespondanceVarNativesVarOptim->NumberOfVariablesOverflowClu[clusterIndex]
                      = -1;
                }
            }
        }

        // ===== HYDRO-CLUSTER-END ===== //
    }

    for (Pays = 0; Pays < ProblemeHebdo->NombreDePays; Pays++)
    {
        if (ProblemeHebdo->CaracteristiquesHydrauliques[Pays]->AccurateWaterValue == OUI_ANTARES)
        {
            ProblemeHebdo->NumeroDeVariableStockFinal[Pays] = NombreDeVariables;
            ProblemeAResoudre->TypeDeVariable[NombreDeVariables] = VARIABLE_NON_BORNEE;
            NombreDeVariables++;

            for (uint nblayer = 0; nblayer < 100; nblayer++)
            {
                ProblemeHebdo->NumeroDeVariableDeTrancheDeStock[Pays][nblayer] = NombreDeVariables;
                ProblemeAResoudre->TypeDeVariable[NombreDeVariables]
                  = VARIABLE_BORNEE_DES_DEUX_COTES;
                NombreDeVariables++;
            }
        }
        else
        {
            ProblemeHebdo->NumeroDeVariableStockFinal[Pays] = -1;
            for (uint nblayer = 0; nblayer < 100; nblayer++)
            {
                ProblemeHebdo->NumeroDeVariableDeTrancheDeStock[Pays][nblayer] = -1;
            }
        }
    }

    // ===== HYDRO-CLUSTERS-START ===== //
    for (Pays = 0; Pays < ProblemeHebdo->NombreDePays; Pays++)
    {
        PALIERS_HYDROCLUSTERS& PaliersHydroclusterDuPays
          = ProblemeHebdo->PaliersHydroclusterDuPays[Pays];
        uint areaClusterCount = PaliersHydroclusterDuPays.areaClusterCount;
        for (int cluster = 0; cluster < areaClusterCount; cluster++)
        {
            auto& clusterHydroData = PaliersHydroclusterDuPays.hydroClusterMap.at(cluster);
            int clusterIndex = PaliersHydroclusterDuPays.clusterIndexTotalCount[cluster];

            if (clusterHydroData.AccurateWaterValue == OUI_ANTARES)
            {
                ProblemeHebdo->NumeroDeVariableStockFinalCluster[clusterIndex] = NombreDeVariables;
                ProblemeAResoudre->TypeDeVariable[NombreDeVariables] = VARIABLE_NON_BORNEE;
                NombreDeVariables++;

                for (uint nblayer = 0; nblayer < 100; nblayer++)
                {
                    ProblemeHebdo->NumeroDeVariableDeTrancheDeStockCluster[clusterIndex][nblayer]
                      = NombreDeVariables;
                    ProblemeAResoudre->TypeDeVariable[NombreDeVariables]
                      = VARIABLE_BORNEE_DES_DEUX_COTES;
                    NombreDeVariables++;
                }
            }
            else
            {
                ProblemeHebdo->NumeroDeVariableStockFinalCluster[clusterIndex] = -1;
                for (uint nblayer = 0; nblayer < 100; nblayer++)
                {
                    ProblemeHebdo->NumeroDeVariableDeTrancheDeStockCluster[clusterIndex][nblayer]
                      = -1;
                }
            }
        }
    }
    // ===== HYDRO-CLUSTER-END ===== //

    ProblemeAResoudre->NombreDeVariables = NombreDeVariables;

    if (ProblemeHebdo->OptimisationAvecCoutsDeDemarrage == OUI_ANTARES)
    {
        Simulation = NON_ANTARES;
        OPT_ConstruireLaListeDesVariablesOptimiseesDuProblemeLineaireCoutsDeDemarrage(ProblemeHebdo,
                                                                                      Simulation);
    }

    return;
}
