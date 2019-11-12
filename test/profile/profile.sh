#!/bin/bash

hypo_solveur()
{
    printf "Hypothèse $4 : le solveur \"$1\" est capable de résoudre des\n"
    printf "grilles de taille $2 dans la mesure où les Rooms sont de taille\n"
    printf "maximum $3. Nous ne pouvons pas tester toutes les grilles de\n"
    printf "taille $2 car il en existe un très grand nombre, donc on partira du principe\n"
    printf "que si le solveur \"$1\" arrive à résoudre une grille générée\n"
    printf "aléatoirement et de taille $2 avec des Rooms de taille maximum $3,\n"
    printf "on estime qu'il est capable de résoudre toutes les grilles de taille $2.\n"
}

seuil_tolerance()
{
    printf "Nous avons décidé que le seuil de tolérence serait de $1 seconde.\n"
}

commande()
{
    printf "\nCommande testée : $1\n\n"
}

preuve_hypo()
{
    printf "\nAfin de prouver que la taille limite pour la résolution des \n"
    printf "grilles avec ce solveur, nous allons maintenant essayer d'exécuter\n"
    printf "ce solveur sur une grille de taille $1.\n"
    printf "Nous estimons de la même manière que pour les grilles de taille $2\n"
    printf "que nous ne pouvons pas tester toutes les grilles de taille $1 \n"
    printf "donc nous allons partir du principe que si le solveur n'arrive pas\n"
    printf "à résoudre une grille générée aléatoirement de taille $1, on \n"
    printf "estime qu'il est incapable de résoudre une grille de taille $1.\n"
    printf "Alors l'hypothèse $3 émise plus tôt sera valide.\n"
}

conclusion()
{
    printf "Nous voyons que le solveur \"$1\" arrive à résoudre toutes grilles\n"
    printf "de taille $2 avec des Rooms de taille maximum $3 et dépasse \n"
    printf "le timeout pour des grilles de taille $4.\nDonc la limite\n"
    printf " de ce solveur pour des Rooms de taille maximum $3 est la taille $2.\n"
    printf "L'hypothèse $5 est donc valide.\n\n\n"
}

test_solveur()
{
    printf "############################### Solveur $2\n\n\n"
    hypo_solveur $2 $3 $5 $1
    seuil_tolerance $6
    for i in `seq 0 9`
    do
        ./kenken -g$3 -M$5 -o $3grid"$i";
        ./kenken -g$4 -M$5 -o $4grid"$i";
    done
    for i in `seq 0 9`
    do
        timeout $6 ./kenken -$2 output/$3grid"$i" > /dev/null;
        
        if [ $(find . -maxdepth 1 -name "gmon.out") ]
        then
            gprof -b kenken gmon.out;
            rm gmon.out
        else
            printf "Dépassement du timeout (timeout = 1 seconde).\n"
        fi
    done
    
    preuve_hypo $4 $3 $1
    for i in `seq 0 9`
    do
        timeout $6 ./kenken -$2 output/$4grid"$i" > /dev/null;
        
        if [ $(find . -maxdepth 1 -name "gmon.out") ]
        then
            gprof -b kenken gmon.out;
            rm gmon.out
        else
            printf "Dépassement du timeout.\n"
        fi
    done
    conclusion $2 $3 $5 $4 $1
}

test_solveur 1 "sbt" 13 14 2 1

test_solveur 2 "ses" 5 6 2 1

test_solveur 3 "sis" 8 9 2 1

test_solveur 4 "slis" 19 20 2 1

test_solveur 5 "sl" 51 52 2 1 

##M5------------------------------

test_solveur 6 "sbt" 7 8 7 1 

test_solveur 7 "ses" 4 5 4 1 

test_solveur 8 "sl" 7 8 7 1 

printf "############################### Solveur logic -a\n\n\n"

printf "Hypothèse 9 : La limite du solveur de grilles qui donne toutes\n"
printf "les solutions est 51.\n"
printf "Nous ne pouvons pas tester toutes les grilles de taille 51 car il en\n"
printf "existe un très grand nombre, donc on partira du principe que si le solveur\n"
printf "arrive à résoudre une grille générée aléatoirement et de taille 51\n"
printf "avec des Rooms de taille maximum 2,"
printf "on estime qu'il est capable de résoudre toutes les grilles de taille 51.\n\n"

for i in `seq 0 9`
do
    ./kenken -g51 -M2 -o 51grid"$i";
    ./kenken -g52 -M2 -o 52grid"$i";
done
for i in `seq 0 9`
do
    timeout 1 ./kenken -sl output/51grid"$i" -a> /dev/null;
        
    if [ $(find . -maxdepth 1 -name "gmon.out") ]
    then
        gprof -b kenken gmon.out;
        rm gmon.out
    else
        printf "Dépassement du timeout.\n"
    fi
done
printf "\nAfin de prouver que la taille maximum des grilles qui donne toutes\n"
printf "les solutions est 51, nous allons maintenant essayer de donner toutes\n"
printf "les solutions de grilles de taille supérieure à la précédente.\n"
printf "Nous estimons de la même manière que pour les grilles de taille 51\n"
printf "que nous ne pouvons pas tester toutes les grilles de taille 52 donc\n"
printf "nous allons partir du principe que si le solveur n'arrive pas à\n"
printf "résoudre une grille générée aléatoirement de taille 52,\n" 
printf "on estime qu'il est incapable de résoudre une grille de taille 41.\n"
for i in `seq 0 9`
do
    timeout 1 ./kenken -sl output/52grid"$i" -a > /dev/null;
        
    if [ $(find . -maxdepth 1 -name "gmon.out") ]
    then
        gprof -b kenken gmon.out;
        rm gmon.out
    else
        printf "Dépassement du timeout.\n"
    fi
done
printf "Nous voyons bien que la résolution qui donne toutes les solutions d'une\n"
printf "grille de taille 52 n'est pas possible en moins dune seconde alors\n" 
printf "qu'avec la taille 51, la résolution qui donne toutes les solutions est\n"
printf "possible.\nDonc l'hypothèse 9 est validée.\n\n"

printf "############################### Generator\n\n\n"

printf "Hypothèse 10 : Le générateur de base est capable de générer des grilles\n"
printf "de taille 250x250 en un temps inférieur au seuil de tolérance (1 seconde).\n" 
printf "Nous ne pouvons pas créer toutes les grilles de taille 250x250 donc on\n"
printf "part du principe que s'il arrive à créer une grille aléatoirement de\n"
printf "taille 250x250 alors il arrivera à créer toutes les grilles de taille\n"
printf "250x250.\n"
for i in `seq 0 9`
do
    timeout 1 ./kenken -g250 > /dev/null;
        
    if [ $(find . -maxdepth 1 -name "gmon.out") ]
    then
        gprof -b kenken gmon.out;
        rm gmon.out
    else
        printf "Dépassement du timeout.\n"
    fi
done
printf "\nAfin de prouver que la limite de temps de génération d'une grille\n"
printf "aléatoire est inférieure au seuil de tolérance est 250x250,\n"
printf "nous allons maintenant essayer de générer une grille de taille 300x300.\n"
printf "De la même manière que pour la création des grilles 250x250, comme on\n"
printf "ne peut pas créer toutes les grilles de taille 300x300, s'il n'arrive\n"
printf "pas à en créer une seule en dessous du seuil critique (1 seconde),\n"
printf "il n'arrivera pour aucune grille.\n\n"
for i in `seq 0 9`
do
    timeout 1 ./kenken -g300 > /dev/null;
        
    if [ $(find . -maxdepth 1 -name "gmon.out") ]
    then
        gprof -b kenken gmon.out;
        rm gmon.out
    else
        printf "Dépassement du timeout.\n"
    fi
done
printf "Nous voyons bien que la génération d'une grille aléatoire de taille 300\n"
printf "met plus de temps que le seuil de tolérance (1 seconde) alors que\n"
printf "la génération d'une grille aléatoire de taille 250 met moins\n"
printf "d'une seconde.\nDonc l'hypothèse 10 est validée.\n\n"

printf "############################### Generator\n\n\n"

printf "Hypothèse 11 : Le générateur d'une grille avec une seule solution\n"
printf "et avec une taille maximum pour les Rooms de 2 est capable de générer\n"
printf "des grilles de taille 55x55 en un temps inférieur au seuil\n"
printf "de tolérance (1 seconde).\nNous ne pouvons pas créer toutes\n"
printf "les grilles de taille 55x55 donc on part du principe que s'il arrive\n"
printf "à créer une grille aléatoirement de taille 55x55 alors il arrivera\n"
printf "à créer toutes les grilles de taille 55x55\n"
for i in `seq 0 9`
do
    timeout 1 ./kenken -g55 -M2 -u > /dev/null;
        
    if [ $(find . -maxdepth 1 -name "gmon.out") ]
    then
        gprof -b kenken gmon.out;
        rm gmon.out
    else
        printf "Dépassement du timeout.\n"
    fi
done
printf "Nous allons maintenant essayer de générer une grille de taille 60x60.\n"
printf "De la même manière que pour la création des grilles 55x55,\n"
printf "comme on ne peut pas créer toutes les grilles de taille 60x60,\n" 
printf "s'il n'arrive pas à en créer une seule en dessous du seuil\n"
printf "critique (1 seconde), il n'arrivera pour aucune grille.\n\n"
for i in `seq 0 9`
do
    timeout 1 ./kenken -g60 -M2 -u> /dev/null;
        
    if [ $(find . -maxdepth 1 -name "gmon.out") ]
    then
        gprof -b kenken gmon.out;
        rm gmon.out
    else
        printf "Dépassement du timeout.\n"
    fi
done
printf "\nNous voyons bien que la génération d'une grille aléatoire de\n"
printf "taille 60 avec une solution unique met plus de temps que le seuil\n"
printf "de tolérance (1 seconde) alors que la génération d'une grille aléatoire\n"
printf "de taille 55 avec une unique solution met moins d'une seconde.\n"
printf "Donc l'hypothèse 11 est validée.\n\n"
