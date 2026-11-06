#include "fichier.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define VERSION_DONNEES 1U
#define NOMBRE_MAX_PATIENTS 100000U

typedef struct {
    int32_t numero;
    char nom[TAILLE_NOM];
    char prenom[TAILLE_NOM];
    int32_t age;
    char symptome[TAILLE_SYMPTOME];
    int32_t gravite;
    int64_t heure_arrivee;
} PatientEnregistre;

static int lire_patient(FILE *fichier, PatientEnregistre *enregistre)
{
    return fread(enregistre, sizeof(*enregistre), 1, fichier) == 1;
}

int fichier_charger(FileAttente *file, int *prochain_numero, const char *chemin)
{
    FILE *fichier;
    char signature[8];
    uint32_t version;
    uint32_t nombre;
    uint32_t numero_suivant;
    uint32_t index;
    FileAttente charge;

    if (file == NULL || prochain_numero == NULL || chemin == NULL) {
        return 0;
    }
    fichier = fopen(chemin, "rb");
    if (fichier == NULL) {
        if (errno == ENOENT) {
            *prochain_numero = 1;
            return 1;
        }
        return 0;
    }
    if (fread(signature, sizeof(signature), 1, fichier) != 1 ||
        memcmp(signature, "UTRIAGE", 8) != 0 ||
        fread(&version, sizeof(version), 1, fichier) != 1 ||
        fread(&nombre, sizeof(nombre), 1, fichier) != 1 ||
        fread(&numero_suivant, sizeof(numero_suivant), 1, fichier) != 1 ||
        version != VERSION_DONNEES || nombre > NOMBRE_MAX_PATIENTS ||
        numero_suivant == 0 || numero_suivant > INT32_MAX) {
        fclose(fichier);
        return 0;
    }

    file_initialiser(&charge);
    for (index = 0; index < nombre; index++) {
        PatientEnregistre enregistre;
        Patient *patient;

        memset(&enregistre, 0, sizeof(enregistre));
        if (!lire_patient(fichier, &enregistre) || enregistre.numero <= 0 ||
            enregistre.age < 0 || enregistre.age > 130 ||
            enregistre.gravite < CRITIQUE || enregistre.gravite > NON_URGENT) {
            file_detruire(&charge);
            fclose(fichier);
            return 0;
        }
        enregistre.nom[TAILLE_NOM - 1] = '\0';
        enregistre.prenom[TAILLE_NOM - 1] = '\0';
        enregistre.symptome[TAILLE_SYMPTOME - 1] = '\0';
        patient = patient_creer(enregistre.numero, enregistre.nom,
                                enregistre.prenom, enregistre.age,
                                enregistre.symptome, (Gravite)enregistre.gravite,
                                (time_t)enregistre.heure_arrivee);
        if (patient == NULL || !file_inserer(&charge, patient)) {
            patient_detruire(patient);
            file_detruire(&charge);
            fclose(fichier);
            return 0;
        }
    }
    if (fclose(fichier) != 0) {
        file_detruire(&charge);
        return 0;
    }
    file_detruire(file);
    *file = charge;
    *prochain_numero = (int)numero_suivant;
    return 1;
}

int fichier_sauvegarder(const FileAttente *file, int prochain_numero,
                        const char *chemin)
{
    char chemin_temporaire[512];
    const Patient *patient;
    FILE *fichier;
    const char signature[8] = "UTRIAGE";
    uint32_t version = VERSION_DONNEES;
    uint32_t nombre;
    uint32_t numero_suivant;
    int succes = 1;

    if (file == NULL || chemin == NULL || prochain_numero < 1 ||
        strlen(chemin) + sizeof(".tmp") > sizeof(chemin_temporaire)) {
        return 0;
    }
    (void)snprintf(chemin_temporaire, sizeof(chemin_temporaire), "%s.tmp", chemin);
    nombre = (uint32_t)file->taille;
    numero_suivant = (uint32_t)prochain_numero;
    fichier = fopen(chemin_temporaire, "wb");
    if (fichier == NULL) {
        return 0;
    }
    succes = fwrite(signature, sizeof(signature), 1, fichier) == 1 &&
             fwrite(&version, sizeof(version), 1, fichier) == 1 &&
             fwrite(&nombre, sizeof(nombre), 1, fichier) == 1 &&
             fwrite(&numero_suivant, sizeof(numero_suivant), 1, fichier) == 1;
    for (patient = file->tete; succes && patient != NULL;
         patient = patient->suivant) {
        PatientEnregistre enregistre;

        memset(&enregistre, 0, sizeof(enregistre));
        enregistre.numero = patient->numero;
        (void)snprintf(enregistre.nom, sizeof(enregistre.nom), "%s", patient->nom);
        (void)snprintf(enregistre.prenom, sizeof(enregistre.prenom), "%s", patient->prenom);
        enregistre.age = patient->age;
        (void)snprintf(enregistre.symptome, sizeof(enregistre.symptome), "%s",
                       patient->symptome);
        enregistre.gravite = patient->gravite;
        enregistre.heure_arrivee = (int64_t)patient->heureArrivee;
        succes = fwrite(&enregistre, sizeof(enregistre), 1, fichier) == 1;
    }
    if (fclose(fichier) != 0) {
        succes = 0;
    }
    if (succes && rename(chemin_temporaire, chemin) == 0) {
        return 1;
    }
    (void)remove(chemin_temporaire);
    return 0;
}

int fichier_ajouter_historique(const Patient *patient, time_t heure_fin,
                               int attente_minutes, const char *chemin)
{
    FILE *fichier;
    int resultat;
    int fermeture;

    if (patient == NULL || chemin == NULL || attente_minutes < 0) {
        return 0;
    }
    fichier = fopen(chemin, "a");
    if (fichier == NULL) {
        return 0;
    }
    resultat = fprintf(fichier, "%d;%s;%s;%d;%s;%d;%lld;%lld;%d\n",
                       patient->numero, patient->nom, patient->prenom,
                       patient->age, patient->symptome, patient->gravite,
                       (long long)patient->heureArrivee,
                       (long long)heure_fin, attente_minutes);
    fermeture = fclose(fichier);
    return resultat >= 0 && fermeture == 0;
}

size_t fichier_parcourir_historique(const char *chemin,
                                    HistoriqueVisiteur visiteur,
                                    void *contexte)
{
    FILE *fichier;
    char ligne[512];
    size_t nombre = 0;

    if (chemin == NULL || visiteur == NULL) {
        return 0;
    }
    fichier = fopen(chemin, "r");
    if (fichier == NULL) {
        return 0;
    }
    while (fgets(ligne, sizeof(ligne), fichier) != NULL) {
        HistoriqueEntree entree;
        int gravite;
        long long arrivee;
        long long fin;

        memset(&entree, 0, sizeof(entree));
        if (sscanf(ligne, "%d;%29[^;];%29[^;];%d;%59[^;];%d;%lld;%lld;%d",
                   &entree.numero, entree.nom, entree.prenom, &entree.age,
                   entree.symptome, &gravite, &arrivee, &fin,
                   &entree.attenteMinutes) != 9 ||
            entree.numero <= 0 || gravite < CRITIQUE || gravite > NON_URGENT ||
            entree.age < 0 || entree.age > 130 ||
            entree.attenteMinutes < 0) {
            continue;
        }
        entree.gravite = (Gravite)gravite;
        entree.heureArrivee = (time_t)arrivee;
        entree.heureFin = (time_t)fin;
        visiteur(&entree, contexte);
        nombre++;
    }
    fclose(fichier);
    return nombre;
}