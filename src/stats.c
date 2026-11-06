#include "stats.h"

#include "fichier.h"

#include <string.h>

#define NOMBRE_SYMPTOMES 100

typedef struct {
    char symptomes[NOMBRE_SYMPTOMES][TAILLE_SYMPTOME];
    int occurrences[NOMBRE_SYMPTOMES];
    int nombreSymptomes;
    long long sommeAges;
    long long sommeAttentes;
    int personnes;
    int premiereConsultation;
    Statistiques *statistiques;
} Accumulateur;

static void ajouter_symptome(Accumulateur *accumulateur, const char *symptome)
{
    int index;

    for (index = 0; index < accumulateur->nombreSymptomes; index++) {
        if (strcmp(accumulateur->symptomes[index], symptome) == 0) {
            accumulateur->occurrences[index]++;
            return;
        }
    }
    if (accumulateur->nombreSymptomes < NOMBRE_SYMPTOMES) {
        index = accumulateur->nombreSymptomes++;
        (void)snprintf(accumulateur->symptomes[index], TAILLE_SYMPTOME,
                       "%s", symptome);
        accumulateur->occurrences[index] = 1;
    }
}

static void ajouter_historique(const HistoriqueEntree *entree, void *contexte)
{
    Accumulateur *accumulateur = contexte;
    Statistiques *statistiques = accumulateur->statistiques;

    accumulateur->personnes++;
    accumulateur->sommeAges += entree->age;
    statistiques->patientsAccueillis++;
    accumulateur->sommeAttentes += entree->attenteMinutes;
    if (accumulateur->premiereConsultation ||
        entree->attenteMinutes < statistiques->attenteMinimum) {
        statistiques->attenteMinimum = entree->attenteMinutes;
    }
    if (accumulateur->premiereConsultation ||
        entree->attenteMinutes > statistiques->attenteMaximum) {
        statistiques->attenteMaximum = entree->attenteMinutes;
    }
    accumulateur->premiereConsultation = 0;
    statistiques->nombreConsultations++;
    ajouter_symptome(accumulateur, entree->symptome);
}

void stats_calculer(const FileAttente *file, const char *chemin_historique,
                    Statistiques *statistiques)
{
    Accumulateur accumulateur;
    const Patient *patient;
    int index;

    memset(statistiques, 0, sizeof(*statistiques));
    memset(&accumulateur, 0, sizeof(accumulateur));
    accumulateur.premiereConsultation = 1;
    accumulateur.statistiques = statistiques;
    if (file != NULL) {
        for (patient = file->tete; patient != NULL; patient = patient->suivant) {
            statistiques->patientsAccueillis++;
            statistiques->patientsEnAttente++;
            if (patient->gravite >= CRITIQUE && patient->gravite <= NON_URGENT) {
                statistiques->parGravite[patient->gravite]++;
            }
            accumulateur.sommeAges += patient->age;
            accumulateur.personnes++;
            ajouter_symptome(&accumulateur, patient->symptome);
        }
    }
    (void)fichier_parcourir_historique(chemin_historique, ajouter_historique,
                                       &accumulateur);
    if (accumulateur.personnes > 0) {
        statistiques->ageMoyen = (double)accumulateur.sommeAges /
                                 accumulateur.personnes;
    }
    if (statistiques->nombreConsultations > 0) {
        statistiques->attenteMoyenne = (double)accumulateur.sommeAttentes /
                                       statistiques->nombreConsultations;
    }
    for (index = 0; index < accumulateur.nombreSymptomes; index++) {
        if (accumulateur.occurrences[index] > statistiques->occurrencesSymptome) {
            statistiques->occurrencesSymptome = accumulateur.occurrences[index];
            (void)snprintf(statistiques->symptomeFrequent,
                           sizeof(statistiques->symptomeFrequent), "%s",
                           accumulateur.symptomes[index]);
        }
    }
}

void stats_afficher(const Statistiques *statistiques, FILE *sortie)
{
    int gravite;

    fprintf(sortie, "\n=== Statistiques ===\n");
    fprintf(sortie, "Patients accueillis : %d\n", statistiques->patientsAccueillis);
    fprintf(sortie, "Patients en attente : %d\n", statistiques->patientsEnAttente);
    fprintf(sortie, "Répartition des patients en attente :\n");
    for (gravite = CRITIQUE; gravite <= NON_URGENT; gravite++) {
        fprintf(sortie, "  %d %-12s : %d ", gravite, gravite_nom((Gravite)gravite),
                statistiques->parGravite[gravite]);
        for (int barre = 0; barre < statistiques->parGravite[gravite]; barre++) {
            fputc('#', sortie);
        }
        fputc('\n', sortie);
    }
    if (statistiques->nombreConsultations > 0) {
        fprintf(sortie, "Attente réelle (min) : moyenne %.1f, min %d, max %d\n",
                statistiques->attenteMoyenne, statistiques->attenteMinimum,
                statistiques->attenteMaximum);
    } else {
        fprintf(sortie, "Attente réelle : aucune consultation enregistrée\n");
    }
    fprintf(sortie, "Âge moyen : %.1f ans\n", statistiques->ageMoyen);
    if (statistiques->occurrencesSymptome > 0) {
        fprintf(sortie, "Symptôme le plus fréquent : %s (%d)\n",
                statistiques->symptomeFrequent, statistiques->occurrencesSymptome);
    }
}