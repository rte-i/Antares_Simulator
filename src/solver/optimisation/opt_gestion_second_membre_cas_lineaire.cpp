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

#include <numeric>

#include "antares/solver/simulation/sim_structure_probleme_economique.h"

double OPT_SommeDesPminThermiques(const PROBLEME_HEBDO*, int, uint);
void OPT_InitialiserLeSecondMembreDuProblemeLineaireCoutsDeDemarrage(PROBLEME_HEBDO*, int, int);

static void shortTermStorageLevelsRHS(
  const std::vector<::ShortTermStorage::AREA_INPUT>& shortTermStorageInput,
  int numberOfAreas,
  std::vector<double>& SecondMembre,
  const CORRESPONDANCES_DES_CONTRAINTES& CorrespondanceCntNativesCntOptim,
  int hourInTheYear)
{
    for (int areaIndex = 0; areaIndex < numberOfAreas; areaIndex++)
    {
        for (auto& storage: shortTermStorageInput[areaIndex])
        {
            const int clusterGlobalIndex = storage.clusterGlobalIndex;
            int cnt = CorrespondanceCntNativesCntOptim
                        .ShortTermStorageLevelConstraint[clusterGlobalIndex];
            SecondMembre[cnt] = storage.series->inflows[hourInTheYear];
        }
    }
}

static void shortTermStorageCumulationRHS(
  const std::vector<::ShortTermStorage::AREA_INPUT>& shortTermStorageInput,
  int numberOfAreas,
  std::vector<double>& SecondMembre,
  const CORRESPONDANCES_DES_CONTRAINTES_HEBDOMADAIRES& CorrespondancesDesContraintesHebdomadaires,
  int weekFirstHour)
{
    for (int areaIndex = 0; areaIndex < numberOfAreas; areaIndex++)
    {
        for (auto& storage: shortTermStorageInput[areaIndex])
        {
            for (const auto& additionalConstraints: storage.additionalConstraints)
            {
                for (const auto& constraint: additionalConstraints.constraints)
                {
                    const int cnt = CorrespondancesDesContraintesHebdomadaires
                                      .ShortTermStorageCumulation[constraint.globalIndex];

                    SecondMembre[cnt] = std::accumulate(
                      constraint.hours.begin(),
                      constraint.hours.end(),
                      0.0,
                      [weekFirstHour, &additionalConstraints](const double sum, const int hour)
                      { return sum + additionalConstraints.rhs[weekFirstHour + hour - 1]; });
                }
            }
        }
    }
}

void OPT_InitialiserLeSecondMembreDuProblemeLineaire(PROBLEME_HEBDO* problemeHebdo,
                                                     int PremierPdtDeLIntervalle,
                                                     int DernierPdtDeLIntervalle,
                                                     int NumeroDeLIntervalle,
                                                     const int optimizationNumber)
{
    int weekFirstHour = problemeHebdo->weekInTheYear * 168;

    const auto& ProblemeAResoudre = problemeHebdo->ProblemeAResoudre;

    std::vector<double>& SecondMembre = ProblemeAResoudre->SecondMembre;

    std::vector<double*>& AdresseOuPlacerLaValeurDesCoutsMarginaux
      = ProblemeAResoudre->AdresseOuPlacerLaValeurDesCoutsMarginaux;

    int NombreDePasDeTempsDUneJournee = problemeHebdo->NombreDePasDeTempsDUneJournee;

    const std::vector<int>& NumeroDeJourDuPasDeTemps = problemeHebdo->NumeroDeJourDuPasDeTemps;
    const std::vector<int>& NumeroDeContrainteEnergieHydraulique
      = problemeHebdo->NumeroDeContrainteEnergieHydraulique;
    const std::vector<int>& NumeroDeContrainteMinEnergieHydraulique
      = problemeHebdo->NumeroDeContrainteMinEnergieHydraulique;
    const std::vector<int>& NumeroDeContrainteMaxEnergieHydraulique
      = problemeHebdo->NumeroDeContrainteMaxEnergieHydraulique;
    const std::vector<int>& NumeroDeContrainteMaxPompage = problemeHebdo
                                                             ->NumeroDeContrainteMaxPompage;

    const std::vector<bool>& DefaillanceNegativeUtiliserConsoAbattue
      = problemeHebdo->DefaillanceNegativeUtiliserConsoAbattue;
    const std::vector<bool>& DefaillanceNegativeUtiliserPMinThermique
      = problemeHebdo->DefaillanceNegativeUtiliserPMinThermique;

    for (int i = 0; i < ProblemeAResoudre->NombreDeContraintes; i++)
    {
        AdresseOuPlacerLaValeurDesCoutsMarginaux[i] = nullptr;
        SecondMembre[i] = 0.0;
    }

    for (int pdtJour = 0, pdtHebdo = PremierPdtDeLIntervalle; pdtHebdo < DernierPdtDeLIntervalle;
         pdtHebdo++, pdtJour++)
    {
        const CORRESPONDANCES_DES_CONTRAINTES& CorrespondanceCntNativesCntOptim
          = problemeHebdo->CorrespondanceCntNativesCntOptim[pdtJour];

        const CONSOMMATIONS_ABATTUES& ConsommationsAbattues = problemeHebdo
                                                                ->ConsommationsAbattues[pdtHebdo];
        const ALL_MUST_RUN_GENERATION& AllMustRunGeneration = problemeHebdo
                                                                ->AllMustRunGeneration[pdtHebdo];
        for (uint32_t pays = 0; pays < problemeHebdo->NombreDePays; pays++)
        {
            int cnt = CorrespondanceCntNativesCntOptim.NumeroDeContrainteDesBilansPays[pays];
            SecondMembre[cnt] = -ConsommationsAbattues.ConsommationAbattueDuPays[pays];

            bool reserveJm1 = (problemeHebdo->YaDeLaReserveJmoins1);
            bool opt1 = (optimizationNumber == PREMIERE_OPTIMISATION);
            if (reserveJm1 && opt1)
            {
                SecondMembre[cnt] -= problemeHebdo->ReserveJMoins1[pays]
                                       .ReserveHoraireJMoins1[pdtHebdo];
            }

            double* adresseDuResultat = &(
              problemeHebdo->ResultatsHoraires[pays].CoutsMarginauxHoraires[pdtHebdo]);
            AdresseOuPlacerLaValeurDesCoutsMarginaux[cnt] = adresseDuResultat;

            cnt = CorrespondanceCntNativesCntOptim
                    .NumeroDeContraintePourEviterLesChargesFictives[pays];
            SecondMembre[cnt] = 0.0;

            double MaxAllMustRunGeneration = 0.0;
            if (AllMustRunGeneration.AllMustRunGenerationOfArea[pays] > 0.0)
            {
                MaxAllMustRunGeneration = AllMustRunGeneration.AllMustRunGenerationOfArea[pays];
            }

            double MaxMoinsConsommationBrute = 0.0;
            if (-(ConsommationsAbattues.ConsommationAbattueDuPays[pays]
                  + AllMustRunGeneration.AllMustRunGenerationOfArea[pays])
                > 0.0)
            {
                MaxMoinsConsommationBrute = -(
                  ConsommationsAbattues.ConsommationAbattueDuPays[pays]
                  + AllMustRunGeneration.AllMustRunGenerationOfArea[pays]);
            }

            SecondMembre[cnt] = DefaillanceNegativeUtiliserConsoAbattue[pays]
                                * (MaxAllMustRunGeneration + MaxMoinsConsommationBrute);

            if (DefaillanceNegativeUtiliserPMinThermique[pays] == 0)
            {
                SecondMembre[cnt] -= OPT_SommeDesPminThermiques(problemeHebdo, pays, pdtHebdo);
            }

            AdresseOuPlacerLaValeurDesCoutsMarginaux[cnt] = nullptr;
        }

        int hourInTheYear = weekFirstHour + pdtHebdo;
        shortTermStorageLevelsRHS(problemeHebdo->ShortTermStorage,
                                  problemeHebdo->NombreDePays,
                                  ProblemeAResoudre->SecondMembre,
                                  CorrespondanceCntNativesCntOptim,
                                  hourInTheYear);
        for (uint32_t interco = 0; interco < problemeHebdo->NombreDInterconnexions; interco++)
        {
            if (const COUTS_DE_TRANSPORT& CoutDeTransport = problemeHebdo->CoutDeTransport[interco];
                CoutDeTransport.IntercoGereeAvecDesCouts)
            {
                int cnt = CorrespondanceCntNativesCntOptim
                            .NumeroDeContrainteDeDissociationDeFlux[interco];
                if (CoutDeTransport.IntercoGereeAvecLoopFlow)
                {
                    SecondMembre[cnt] = problemeHebdo->ValeursDeNTC[pdtHebdo]
                                          .ValeurDeLoopFlowOrigineVersExtremite[interco];
                }
                else
                {
                    SecondMembre[cnt] = 0.;
                }
                AdresseOuPlacerLaValeurDesCoutsMarginaux[cnt] = nullptr;
            }
        }

        for (uint32_t cntCouplante = 0; cntCouplante < problemeHebdo->NombreDeContraintesCouplantes;
             cntCouplante++)
        {
            const CONTRAINTES_COUPLANTES& MatriceDesContraintesCouplantes
              = problemeHebdo->MatriceDesContraintesCouplantes[cntCouplante];
            if (MatriceDesContraintesCouplantes.TypeDeContrainteCouplante != CONTRAINTE_HORAIRE)
            {
                continue;
            }

            int cnt = CorrespondanceCntNativesCntOptim
                        .NumeroDeContrainteDesContraintesCouplantes[cntCouplante];
            if (cnt >= 0)
            {
                SecondMembre[cnt] = MatriceDesContraintesCouplantes
                                      .SecondMembreDeLaContrainteCouplante[pdtHebdo];
                AdresseOuPlacerLaValeurDesCoutsMarginaux[cnt] = problemeHebdo
                                                                  ->ResultatsContraintesCouplantes
                                                                    [MatriceDesContraintesCouplantes
                                                                       .bindingConstraint]
                                                                  .data()
                                                                + pdtHebdo;
            }
        }
    }

    for (int pdtHebdo = PremierPdtDeLIntervalle; pdtHebdo < DernierPdtDeLIntervalle;)
    {
        int jour = NumeroDeJourDuPasDeTemps[pdtHebdo];
        int indexCorrespondanceCnt = (!problemeHebdo->OptimisationAuPasHebdomadaire) ? 0 : jour;

        CORRESPONDANCES_DES_CONTRAINTES_JOURNALIERES& CorrespondanceCntNativesCntOptimJournalieres
          = problemeHebdo->CorrespondanceCntNativesCntOptimJournalieres[indexCorrespondanceCnt];

        for (uint32_t cntCouplante = 0; cntCouplante < problemeHebdo->NombreDeContraintesCouplantes;
             cntCouplante++)
        {
            const CONTRAINTES_COUPLANTES& MatriceDesContraintesCouplantes
              = problemeHebdo->MatriceDesContraintesCouplantes[cntCouplante];
            if (MatriceDesContraintesCouplantes.TypeDeContrainteCouplante == CONTRAINTE_JOURNALIERE)
            {
                int cnt = CorrespondanceCntNativesCntOptimJournalieres
                            .NumeroDeContrainteDesContraintesCouplantes[cntCouplante];
                if (cnt >= 0)
                {
                    SecondMembre[cnt] = MatriceDesContraintesCouplantes
                                          .SecondMembreDeLaContrainteCouplante[jour];
                    AdresseOuPlacerLaValeurDesCoutsMarginaux[cnt]
                      = problemeHebdo
                          ->ResultatsContraintesCouplantes[MatriceDesContraintesCouplantes
                                                             .bindingConstraint]
                          .data()
                        + jour;
                }
            }
        }
        pdtHebdo += NombreDePasDeTempsDUneJournee;
    }

    if (problemeHebdo->NombreDePasDeTempsPourUneOptimisation
        > problemeHebdo->NombreDePasDeTempsDUneJournee)
    {
        const CORRESPONDANCES_DES_CONTRAINTES_HEBDOMADAIRES&
          CorrespondanceCntNativesCntOptimHebdomadaires
          = problemeHebdo->CorrespondanceCntNativesCntOptimHebdomadaires;

        for (uint32_t cntCouplante = 0; cntCouplante < problemeHebdo->NombreDeContraintesCouplantes;
             cntCouplante++)
        {
            const CONTRAINTES_COUPLANTES& MatriceDesContraintesCouplantes
              = problemeHebdo->MatriceDesContraintesCouplantes[cntCouplante];

            if (MatriceDesContraintesCouplantes.TypeDeContrainteCouplante
                != CONTRAINTE_HEBDOMADAIRE)
            {
                continue;
            }

            int cnt = CorrespondanceCntNativesCntOptimHebdomadaires
                        .NumeroDeContrainteDesContraintesCouplantes[cntCouplante];
            if (cnt >= 0)
            {
                SecondMembre[cnt] = MatriceDesContraintesCouplantes
                                      .SecondMembreDeLaContrainteCouplante[0];
                AdresseOuPlacerLaValeurDesCoutsMarginaux[cnt] = problemeHebdo
                                                                  ->ResultatsContraintesCouplantes
                                                                    [MatriceDesContraintesCouplantes
                                                                       .bindingConstraint]
                                                                  .data();
            }
        }
    }

    for (uint32_t pays = 0; pays < problemeHebdo->NombreDePays; pays++)
    {
        int cnt = NumeroDeContrainteEnergieHydraulique[pays];
        if (cnt >= 0)
        {
            SecondMembre[cnt] = problemeHebdo->CaracteristiquesHydrauliques[pays]
                                  .CntEnergieH2OParIntervalleOptimise[NumeroDeLIntervalle];
            AdresseOuPlacerLaValeurDesCoutsMarginaux[cnt] = nullptr;
        }
    }

    for (uint32_t pays = 0; pays < problemeHebdo->NombreDePays; pays++)
    {
        bool presenceHydro = problemeHebdo->CaracteristiquesHydrauliques[pays]
                               .PresenceDHydrauliqueModulable;
        bool TurbEntreBornes = problemeHebdo->CaracteristiquesHydrauliques[pays]
                                 .TurbinageEntreBornes;
        if (presenceHydro
            && (TurbEntreBornes
                || problemeHebdo->CaracteristiquesHydrauliques[pays].PresenceDePompageModulable))
        {
            int cnt = NumeroDeContrainteMinEnergieHydraulique[pays];
            if (cnt >= 0)
            {
                SecondMembre[cnt] = problemeHebdo->CaracteristiquesHydrauliques[pays]
                                      .MinEnergieHydrauParIntervalleOptimise[NumeroDeLIntervalle];
                AdresseOuPlacerLaValeurDesCoutsMarginaux[cnt] = nullptr;
            }
        }
    }

    for (uint32_t pays = 0; pays < problemeHebdo->NombreDePays; pays++)
    {
        bool presenceHydro = problemeHebdo->CaracteristiquesHydrauliques[pays]
                               .PresenceDHydrauliqueModulable;
        bool TurbEntreBornes = problemeHebdo->CaracteristiquesHydrauliques[pays]
                                 .TurbinageEntreBornes;
        if (presenceHydro
            && (TurbEntreBornes
                || problemeHebdo->CaracteristiquesHydrauliques[pays].PresenceDePompageModulable))
        {
            int cnt = NumeroDeContrainteMaxEnergieHydraulique[pays];
            if (cnt >= 0)
            {
                SecondMembre[cnt] = problemeHebdo->CaracteristiquesHydrauliques[pays]
                                      .MaxEnergieHydrauParIntervalleOptimise[NumeroDeLIntervalle];
                AdresseOuPlacerLaValeurDesCoutsMarginaux[cnt] = nullptr;
            }
        }
    }

    for (uint32_t pays = 0; pays < problemeHebdo->NombreDePays; pays++)
    {
        if (problemeHebdo->CaracteristiquesHydrauliques[pays].PresenceDePompageModulable)
        {
            int cnt = NumeroDeContrainteMaxPompage[pays];
            if (cnt >= 0)
            {
                SecondMembre[cnt] = problemeHebdo->CaracteristiquesHydrauliques[pays]
                                      .MaxEnergiePompageParIntervalleOptimise[NumeroDeLIntervalle];
                AdresseOuPlacerLaValeurDesCoutsMarginaux[cnt] = nullptr;
            }
        }
    }

    for (int pdtJour = 0, pdtHebdo = PremierPdtDeLIntervalle; pdtHebdo < DernierPdtDeLIntervalle;
         pdtHebdo++, pdtJour++)
    {
        const CORRESPONDANCES_DES_CONTRAINTES& CorrespondanceCntNativesCntOptim
          = problemeHebdo->CorrespondanceCntNativesCntOptim[pdtJour];

        for (uint32_t pays = 0; pays < problemeHebdo->NombreDePays; pays++)
        {
            if (!problemeHebdo->CaracteristiquesHydrauliques[pays].SuiviNiveauHoraire)
            {
                continue;
            }

            int cnt = CorrespondanceCntNativesCntOptim.NumeroDeContrainteDesNiveauxPays[pays];
            if (cnt >= 0)
            {
                SecondMembre[cnt] = problemeHebdo->CaracteristiquesHydrauliques[pays]
                                      .ApportNaturelHoraire[pdtHebdo];
                if (pdtHebdo == 0)
                {
                    SecondMembre[cnt] += problemeHebdo->CaracteristiquesHydrauliques[pays]
                                           .NiveauInitialReservoir;
                }
                AdresseOuPlacerLaValeurDesCoutsMarginaux[cnt] = nullptr;
            }
        }
    }

    for (uint32_t pays = 0; pays < problemeHebdo->NombreDePays; pays++)
    {
        if (problemeHebdo->CaracteristiquesHydrauliques[pays].AccurateWaterValue
            && problemeHebdo->CaracteristiquesHydrauliques[pays].DirectLevelAccess)
        {
            int cnt = problemeHebdo->NumeroDeContrainteEquivalenceStockFinal[pays];
            if (cnt >= 0)
            {
                SecondMembre[cnt] = 0;

                AdresseOuPlacerLaValeurDesCoutsMarginaux[cnt] = nullptr;
            }
        }
        if (problemeHebdo->CaracteristiquesHydrauliques[pays].AccurateWaterValue)
        {
            int cnt = problemeHebdo->NumeroDeContrainteExpressionStockFinal[pays];
            if (cnt >= 0)
            {
                SecondMembre[cnt] = 0;

                AdresseOuPlacerLaValeurDesCoutsMarginaux[cnt] = nullptr;
            }
        }
    }
    shortTermStorageCumulationRHS(problemeHebdo->ShortTermStorage,
                                  problemeHebdo->NombreDePays,
                                  ProblemeAResoudre->SecondMembre,
                                  problemeHebdo->CorrespondanceCntNativesCntOptimHebdomadaires,
                                  weekFirstHour);
    if (problemeHebdo->OptimisationAvecCoutsDeDemarrage)
    {
        OPT_InitialiserLeSecondMembreDuProblemeLineaireCoutsDeDemarrage(problemeHebdo,
                                                                        PremierPdtDeLIntervalle,
                                                                        DernierPdtDeLIntervalle);
    }

    return;
}
