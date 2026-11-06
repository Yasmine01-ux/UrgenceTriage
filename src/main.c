#include "fichier.h"
#include "file.h"
#include "outils.h"
#include "stats.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define FICHIER_ATTENTE "data/file_attente.dat"
#define FICHIER_HISTORIQUE "data/historique.csv"
#define FICHIER_RAPPORT "data/rapport_jour.txt"

static void afficher_patient(const Patient *patient)
{
    char date[32];

    outil_afficher_date(patient->heureArrivee, date, sizeof(date));
    printf("#%d | %s %s | %d ans | %s | %s\n", patient->numero,
           patient->prenom, patient->nom, patient->age,
           gravite_nom(patient->gravite), patient->symptome);
    printf("  Arrivée : %s | Délai cible : ", date);
    if (gravite_delai_cible(patient->gravite) == 0) {
        printf("immédiat\n");
    } else {
        printf("%d min\n", gravite_delai_cible(patient->gravite));
    }
}

static void afficher_file(const FileAttente *file)
{
    const Patient *patient;
    int position = 1;

    if (file->tete == NULL) {
        printf("La file d'attente est vide.\n");
        return;
    }
    printf("POS | N°  | NOM / PRÉNOM          | ÂGE | GRAVITÉ       | ATTENTE ESTIMÉE\n");
    for (patient = file->tete; patient != NULL; patient = patient->suivant) {
        int attente = file_estimer_attente(file, patient);
        int delai = gravite_delai_cible(patient->gravite);

        printf("%3d | %-3d | %-22s | %3d | %-13s | %4d min",
               position++, patient->numero, patient->nom, patient->age,
               gravite_nom(patient->gravite), attente);
        if (delai > 0 && attente > delai) {
            printf("  ALERTE: délai cible dépassé");
        }
        printf("\n");
    }
}

static int ajouter_patient(FileAttente *file, int *prochain_numero)
{
    char nom[TAILLE_NOM];
    char prenom[TAILLE_NOM];
    char symptome[TAILLE_SYMPTOME];
    int age;
    int gravite;
    Patient *patient;

    if (!outil_lire_texte("Nom : ", nom, sizeof(nom)) ||
        !outil_lire_texte("Prénom : ", prenom, sizeof(prenom)) ||
        !outil_lire_entier("Âge (0-130) : ", 0, 130, &age) ||
        !outil_lire_texte("Symptôme : ", symptome, sizeof(symptome)) ||
        !outil_lire_entier("Gravité (1=critique, 5=non urgent) : ",
                           CRITIQUE, NON_URGENT, &gravite)) {
        return 0;
    }
    if (*prochain_numero < 1 || *prochain_numero == INT_MAX) {
        fprintf(stderr, "Limite de numérotation atteinte.\n");
        return 0;
    }
    patient = patient_creer(*prochain_numero, nom, prenom, age, symptome,
                            (Gravite)gravite, time(NULL));
    if (patient == NULL || !file_inserer(file, patient)) {
        patient_detruire(patient);
        fprintf(stderr, "Mémoire insuffisante : patient non ajouté.\n");
        return 0;
    }
    (*prochain_numero)++;
    printf("Patient #%d ajouté à la file.\n", patient->numero);
    return 1;
}

static void appeler_patient(FileAttente *file)
{
    Patient *patient = file_appeler(file);
    time_t heure_fin;
    double minutes;
    int attente;

    if (patient == NULL) {
        printf("Aucun patient en attente.\n");
        return;
    }
    heure_fin = time(NULL);
    minutes = difftime(heure_fin, patient->heureArrivee) / 60.0;
    attente = minutes > 0.0 ? (int)minutes : 0;
    if (!fichier_ajouter_historique(patient, heure_fin, attente,
                                    FICHIER_HISTORIQUE)) {
        (void)file_inserer(file, patient);
        fprintf(stderr, "Historique inaccessible : le patient reste en attente.\n");
        return;
    }
    printf("Patient appelé : ");
    afficher_patient(patient);
    printf("Temps écoulé depuis l'arrivée : %d min.\n", attente);
    patient_detruire(patient);
}

static void rechercher_patient(const FileAttente *file)
{
    int mode;
    Patient *patient = NULL;

    if (!outil_lire_entier("Rechercher par (1) numéro ou (2) nom : ", 1, 2,
                           &mode)) {
        return;
    }
    if (mode == 1) {
        int numero;
        if (outil_lire_entier("Numéro : ", 1, 2147483647, &numero)) {
            patient = file_rechercher_numero(file, numero);
        }
    } else {
        char nom[TAILLE_NOM];
        if (outil_lire_texte("Nom ou prénom (recherche partielle) : ", nom,
                             sizeof(nom))) {
            patient = file_rechercher_nom(file, nom);
        }
    }
    if (patient == NULL) {
        printf("Aucun patient correspondant dans la file d'attente.\n");
    } else {
        afficher_patient(patient);
    }
}

static void modifier_gravite(FileAttente *file)
{
    int numero;
    int gravite;
    Patient *patient;

    if (!outil_lire_entier("Numéro du patient : ", 1, 2147483647, &numero)) {
        return;
    }
    patient = file_rechercher_numero(file, numero);
    if (patient == NULL) {
        printf("Patient #%d introuvable dans la file.\n", numero);
        return;
    }
    printf("Gravité actuelle : %s\n", gravite_nom(patient->gravite));
    if (outil_lire_entier("Nouvelle gravité (1-5) : ", CRITIQUE, NON_URGENT,
                          &gravite) &&
        file_modifier_gravite(file, numero, (Gravite)gravite)) {
        printf("Gravité mise à jour; la file a été réordonnée.\n");
    }
}

static void supprimer_patient(FileAttente *file)
{
    int numero;

    if (!outil_lire_entier("Numéro du patient à retirer : ", 1, 2147483647,
                           &numero)) {
        return;
    }
    if (file_supprimer(file, numero)) {
        printf("Patient #%d retiré de la file.\n", numero);
    } else {
        printf("Patient #%d introuvable dans la file.\n", numero);
    }
}

static void afficher_statistiques(const FileAttente *file)
{
    Statistiques statistiques;

    stats_calculer(file, FICHIER_HISTORIQUE, &statistiques);
    stats_afficher(&statistiques, stdout);
}

static void exporter_rapport(const FileAttente *file)
{
    FILE *rapport = fopen(FICHIER_RAPPORT, "w");
    Statistiques statistiques;

    if (rapport == NULL) {
        fprintf(stderr, "Impossible d'écrire le rapport %s.\n", FICHIER_RAPPORT);
        return;
    }
    stats_calculer(file, FICHIER_HISTORIQUE, &statistiques);
    fprintf(rapport, "RAPPORT DE FIN DE JOURNÉE - URGENCETRIAGE\n");
    fprintf(rapport, "Généré le : %s\n", ctime(&(time_t){time(NULL)}));
    stats_afficher(&statistiques, rapport);
    fprintf(rapport, "\nPatients encore en attente :\n");
    if (file->tete == NULL) {
        fprintf(rapport, "Aucun\n");
    }
    for (const Patient *patient = file->tete; patient != NULL;
         patient = patient->suivant) {
        fprintf(rapport, "#%d;%s;%s;%d;%s;%s\n", patient->numero,
                patient->nom, patient->prenom, patient->age,
                gravite_nom(patient->gravite), patient->symptome);
    }
    if (fclose(rapport) != 0) {
        fprintf(stderr, "Erreur lors de l'écriture du rapport.\n");
        return;
    }
    printf("Rapport exporté dans %s.\n", FICHIER_RAPPORT);
}

static void afficher_menu(const FileAttente *file)
{
    printf("\n========== URGENCE TRIAGE ==========\n");
    printf("Patients en attente : %d\n", file->taille);
    if (file->tete != NULL) {
        printf("Prochain patient : #%d %s %s (gravité %d)\n",
               file->tete->numero, file->tete->prenom, file->tete->nom,
               file->tete->gravite);
    }
    printf("------------------------------------\n");
    printf("1. Enregistrer un nouveau patient\n");
    printf("2. Afficher la file d'attente\n");
    printf("3. Appeler le prochain patient\n");
    printf("4. Rechercher un patient\n");
    printf("5. Modifier la gravité d'un patient\n");
    printf("6. Statistiques de la journée\n");
    printf("7. Exporter le rapport\n");
    printf("8. Retirer un patient de la file\n");
    printf("0. Sauvegarder et quitter\n");
}

int main(void)
{
    FileAttente file;
    int prochain_numero = 1;
    int choix;
    int continuer = 1;

    file_initialiser(&file);
    if (!fichier_charger(&file, &prochain_numero, FICHIER_ATTENTE)) {
        fprintf(stderr, "Fichier de sauvegarde invalide ou inaccessible; démarrage avec une file vide.\n");
    }
    printf("UrgenceTriage - simulation pédagogique, pas un outil médical.\n");
    while (continuer) {
        afficher_menu(&file);
        if (!outil_lire_entier("Votre choix : ", 0, 8, &choix)) {
            printf("\nFin de l'entrée; sauvegarde en cours.\n");
            break;
        }
        switch (choix) {
        case 1:
            (void)ajouter_patient(&file, &prochain_numero);
            break;
        case 2:
            afficher_file(&file);
            break;
        case 3:
            appeler_patient(&file);
            break;
        case 4:
            rechercher_patient(&file);
            break;
        case 5:
            modifier_gravite(&file);
            break;
        case 6:
            afficher_statistiques(&file);
            break;
        case 7:
            exporter_rapport(&file);
            break;
        case 8:
            supprimer_patient(&file);
            break;
        case 0:
            continuer = 0;
            break;
        }
    }
    if (!fichier_sauvegarder(&file, prochain_numero, FICHIER_ATTENTE)) {
        fprintf(stderr, "Échec de la sauvegarde dans %s.\n", FICHIER_ATTENTE);
    }
    file_detruire(&file);
    printf("Au revoir.\n");
    return 0;
}