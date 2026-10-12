#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <cassert>
#include <queue>
#include <map>
#include <tuple>
#include <cstdlib>

/**
 * Représente un état de l'agent dans la recherche A*.
 *
 * Un état est défini par la position de l'agent et par
 * l'ensemble des lingots déjà ramassés, représenté par un bitmask.
 */
struct Etat
{
    int ligne;
    int colonne;
    unsigned int lingotsRamasse;
};

/**
 * Représente une transition possible depuis un état.
 *
 * etat   : nouvel état obtenu après l'action
 * action : action effectuée (h, b, g, d ou s)
 * cout   : coût de cette transition
 */
struct Successeur
{
    Etat etat;
    char action;
    int cout;
};

/**
 * Vérifie si une case peut être visitée par l'agent.
 *
 * Une case est invalide si elle est hors de la grille,
 * contient un puits ou contient un Wumpus.
 *
 * @return true si la case est accessible, false sinon.
 */
bool estCaseValide(
    int ligne,
    int colonne,
    const std::vector<std::vector<bool>> &puits,
    const std::vector<std::vector<bool>> &wumpus)
{
    if (ligne < 0 || ligne >= static_cast<int>(puits.size()))
    {
        return false;
    }
    else if (colonne < 0 || colonne >= static_cast<int>(puits[ligne].size()))
    {
        return false;
    }
    else if (puits[ligne][colonne])
    {
        return false;
    }
    else if (wumpus[ligne][colonne])
    {
        return false;
    }
    else
    {
        return true;
    }
}

/**
 * Génère toutes les actions possibles à partir d'un état.
 *
 * Les déplacements possibles sont haut, bas, gauche et droite.
 * Si l'agent se trouve sur un lingot non encore ramassé,
 * l'action 's' est également ajoutée.
 *
 * @return la liste des successeurs accessibles.
 */
std::vector<Successeur> genererSuccesseur(
    const Etat &etat,
    const std::vector<std::vector<bool>> &puits,
    const std::vector<std::vector<bool>> &wumpus,
    const std::vector<std::vector<int>> &poids,
    const std::vector<std::vector<bool>> &lingots,
    const std::vector<std::vector<int>> &idLingots)
{
    std::vector<Successeur> successeur;

    // Haut
    int nouvelleLigne = etat.ligne - 1;
    int nouvelleColonne = etat.colonne;

    if (estCaseValide(nouvelleLigne, nouvelleColonne, puits, wumpus))
    {
        Etat nouvelEtat;

        nouvelEtat.ligne = nouvelleLigne;
        nouvelEtat.colonne = nouvelleColonne;
        nouvelEtat.lingotsRamasse = etat.lingotsRamasse;

        Successeur s;

        s.etat = nouvelEtat;
        s.action = 'h';
        s.cout = poids[nouvelleLigne][nouvelleColonne];

        successeur.push_back(s);
    }

    // Bas
    nouvelleLigne = etat.ligne + 1;
    nouvelleColonne = etat.colonne;

    if (estCaseValide(nouvelleLigne, nouvelleColonne, puits, wumpus))
    {
        Etat nouvelEtat;
        nouvelEtat.ligne = nouvelleLigne;
        nouvelEtat.colonne = nouvelleColonne;
        nouvelEtat.lingotsRamasse = etat.lingotsRamasse;

        Successeur s;
        s.etat = nouvelEtat;
        s.action = 'b';
        s.cout = poids[nouvelleLigne][nouvelleColonne];

        successeur.push_back(s);
    }

    // Gauche
    nouvelleLigne = etat.ligne;
    nouvelleColonne = etat.colonne - 1;

    if (estCaseValide(nouvelleLigne, nouvelleColonne, puits, wumpus))
    {
        Etat nouvelEtat;
        nouvelEtat.ligne = nouvelleLigne;
        nouvelEtat.colonne = nouvelleColonne;
        nouvelEtat.lingotsRamasse = etat.lingotsRamasse;

        Successeur s;
        s.etat = nouvelEtat;
        s.action = 'g';
        s.cout = poids[nouvelleLigne][nouvelleColonne];

        successeur.push_back(s);
    }

    // Droite
    nouvelleLigne = etat.ligne;
    nouvelleColonne = etat.colonne + 1;

    if (estCaseValide(nouvelleLigne, nouvelleColonne, puits, wumpus))
    {
        Etat nouvelEtat;
        nouvelEtat.ligne = nouvelleLigne;
        nouvelEtat.colonne = nouvelleColonne;
        nouvelEtat.lingotsRamasse = etat.lingotsRamasse;

        Successeur s;
        s.etat = nouvelEtat;
        s.action = 'd';
        s.cout = poids[nouvelleLigne][nouvelleColonne];

        successeur.push_back(s);
    }

    // Action saisir
    if (lingots[etat.ligne][etat.colonne])
    {
        int id = idLingots[etat.ligne][etat.colonne];

        // Crée un masque contenant uniquement le bit associé a ce lingot.
        unsigned int masque = 1u << id;

        if ((etat.lingotsRamasse & masque) == 0)
        {
            Etat nouvelEtat = etat;

            // Active le bit correspondant au lingot ramassé.
            nouvelEtat.lingotsRamasse =
                etat.lingotsRamasse | masque;

            Successeur s;

            s.etat = nouvelEtat;
            s.action = 's';
            s.cout = 0;
            successeur.push_back(s);
        }
    }

    return successeur;
}

/**
 * Représente un noeud utilisé par l'algorithme A*.
 *
 * g : coût réel depuis l'état initial
 * h : estimation du coût restant
 * actions : suite d'actions permettant d'atteindre cet état
 */
struct Noeud
{
    Etat etat;

    int g;
    int h;

    std::string actions;
};

/**
 * Compare deux noeuds selon leur valeur f(n) = g(n) + h(n).
 *
 * La priority_queue doit donner priorité au noeud ayant
 * la plus petite valeur de f.
 */
struct ComparerNoeud
{
    bool operator()(const Noeud &a, const Noeud &b) const
    {
        return (a.g + a.h) > (b.g + b.h);
    }
};

/**
 * Calcule une heuristique admissible pour A*.
 *
 * L'heuristique correspond à la plus grande distance de Manhattan
 * entre la position actuelle de l'agent et un lingot restant.
 *
 * Les obstacles et les poids supérieurs à 1 sont ignorés,
 * ce qui permet de ne pas surestimer le coût réel restant.
 */
int heuristique(
    const Etat &etat,
    const std::vector<std::vector<bool>> &lingots,
    const std::vector<std::vector<int>> &idLingots)
{
    int maxDistance = 0;

    for (int i = 0; i < static_cast<int>(lingots.size()); i++)
    {
        for (int j = 0; j < static_cast<int>(lingots[i].size()); j++)
        {
            if (lingots[i][j])
            {
                int id = idLingots[i][j];
                unsigned int masque = 1u << id;

                if ((masque & etat.lingotsRamasse) == 0)
                {
                    int distance =
                        std::abs(etat.ligne - i) +
                        std::abs(etat.colonne - j);

                    if (distance > maxDistance)
                    {
                        maxDistance = distance;
                    }
                }
            }
        }
    }
    return maxDistance;
}

int main(int argc, char *argv[])
{
    // Vérifie que le chemin vers la carte est fourni en argument
    if (argc != 2)
    {
        std::cout << "Utilisation: " << argv[0] << " carte.txt" << std::endl;
        return 0;
    }

    // Ouvre le fichier contenant la carte.
    std::ifstream file(argv[1]);
    std::string line;

    // Structure utilisées pour représenter les différents éléments de la carte.
    std::vector<std::vector<bool>> lingots;
    std::vector<std::vector<bool>> puits;
    std::vector<std::vector<bool>> wumpus;
    std::vector<std::vector<int>> poids;
    std::vector<std::vector<int>> idLingots;
    int nombreLingots = 0;

    // Lecture du fichier et construction des différentes grilles.
    while (std::getline(file, line))
    {
        lingots.push_back(std::vector<bool>());
        puits.push_back(std::vector<bool>());
        wumpus.push_back(std::vector<bool>());
        poids.push_back(std::vector<int>());
        idLingots.push_back(std::vector<int>());
        for (char c : line)
        {
            if (c == '$')
            {
                lingots.back().push_back(true);
                puits.back().push_back(false);
                wumpus.back().push_back(false);
                poids.back().push_back(1);

                // Attribue un identifiant unique à chaque lingot.
                idLingots.back().push_back(nombreLingots);
                nombreLingots++;
            }
            else if (c == 'W')
            {
                lingots.back().push_back(false);
                puits.back().push_back(false);
                wumpus.back().push_back(true);
                poids.back().push_back(0);

                idLingots.back().push_back(-1);
            }
            else if (c == 'P')
            {
                lingots.back().push_back(false);
                puits.back().push_back(true);
                wumpus.back().push_back(false);
                poids.back().push_back(0);

                idLingots.back().push_back(-1);
            }
            else if (c >= '0' && c <= '9')
            {
                lingots.back().push_back(false);
                puits.back().push_back(false);
                wumpus.back().push_back(false);
                poids.back().push_back(c - '0');

                idLingots.back().push_back(-1);
            }
            else
            {
                std::cout << "Erreur format fichier. Le caractère '" << c << "' non reconnu." << std::endl;
                return -1;
            }
        }
    }

    // État initial : L'agent commence en (0,0) sans lingot ramassé.
    Etat depart;
    depart.ligne = 0;
    depart.colonne = 0;
    depart.lingotsRamasse = 0;

    // Création du premier noeud de la recherche A*
    Noeud noeudDepart;

    noeudDepart.etat = depart;
    noeudDepart.g = 0;
    noeudDepart.h = heuristique(
        depart,
        lingots,
        idLingots);

    noeudDepart.actions = "";

    // File de priorité des noeuds explorer.
    // Le noeud ayant la plus petite valeur f = g + h est prioritaire.
    std::priority_queue<
        Noeud,
        std::vector<Noeud>,
        ComparerNoeud>
        ouverts;

    // Mémorise le meilleur coût connu pour chaque état.
    std::map<std::tuple<int, int, unsigned int>, int> meilleurCout;

    meilleurCout[std::make_tuple(
        depart.ligne,
        depart.colonne,
        depart.lingotsRamasse)] = 0;
    ouverts.push(noeudDepart);

    // Bitmask représentant l'état où tous les lingots ont été ramassés.
    unsigned int objectifLingots = (1u << nombreLingots) - 1;

    while (!ouverts.empty())
    {
        // Sélectionne le noeud ayant la meilleure valeur f = g + h.
        Noeud courant = ouverts.top();
        ouverts.pop();

        auto cleCourante = std::make_tuple(
            courant.etat.ligne,
            courant.etat.colonne,
            courant.etat.lingotsRamasse);

        // Ignore un ancien noeud si un chemin moins coûteux vers
        // le même état a déjà été trouvé.
        if (courant.g > meilleurCout[cleCourante])
        {
            continue;
        }

        // Condition d'objectif : tous les lingots ont été ramassés.
        if (courant.etat.lingotsRamasse == objectifLingots)
        {
            std::cout << courant.actions << "q" << std::endl;
            return 0;
        }

        // Génère tous les déplacements et actions possibles depuis l'état courant.
        std::vector<Successeur> successeurs =
            genererSuccesseur(
                courant.etat,
                puits,
                wumpus,
                poids,
                lingots,
                idLingots);

        // Évalue chacun des successeurs générés.
        for (const Successeur &succ : successeurs)
        {
            // Calcule le nouveau coût réel g.
            int nouveauG = courant.g + succ.cout;

            auto cle = std::make_tuple(
                succ.etat.ligne,
                succ.etat.colonne,
                succ.etat.lingotsRamasse);

            // Conserve cet état seulement s'il est nouveau ou si
            // un chemin moins coûteux vient d'être découvert.
            if (meilleurCout.find(cle) == meilleurCout.end() || nouveauG < meilleurCout[cle])
            {
                meilleurCout[cle] = nouveauG;

                Noeud nouveauNoeud;

                nouveauNoeud.etat = succ.etat;
                nouveauNoeud.g = nouveauG;

                // Calcule l'estimation h du coût restant.
                nouveauNoeud.h = heuristique(
                    nouveauNoeud.etat,
                    lingots,
                    idLingots);

                // Ajoute l'action l'action courante à la séquence déjà effectuée.
                nouveauNoeud.actions =
                    courant.actions + succ.action;

                // Ajoute le nouveau noeud à la file de priorité
                ouverts.push(nouveauNoeud);
            }
        }
    }

    // Aucun chemin permettant de ramasser tous les lingots n'a été trouvé.
    std::cout << "q" << std::endl;
}
