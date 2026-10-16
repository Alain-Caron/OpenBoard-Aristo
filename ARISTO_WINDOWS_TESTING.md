# Équerre Aristo V2 — compilation et recette Windows

Cette branche ajoute un instrument natif `openboardtool://aristo` à OpenBoard.
Elle part de la branche stable `master` (OpenBoard 1.7.7 au moment de la
modification) et utilise Qt 6/C++17 comme le projet amont.

## Compiler sous Windows

1. Installer Visual Studio 2022 avec **Desktop development with C++**, CMake,
   Ninja et la version de Qt 6 utilisée par OpenBoard.
2. Installer les dépendances indiquées dans le guide officiel OpenBoard :
   Poppler, OpenSSL, QuaZip, FFmpeg et zlib.
3. Ouvrir un terminal `x64 Native Tools Command Prompt for VS 2022`.
4. Configurer puis compiler depuis la racine du dépôt :

```powershell
cmake -S . -B build-windows -G Ninja -DQT_VERSION=6 -DCMAKE_BUILD_TYPE=Release
cmake --build build-windows --config Release
```

Les chemins propres aux dépendances peuvent être fournis avec les variables
CMake documentées par OpenBoard ou via `CMAKE_PREFIX_PATH`.

## Recette fonctionnelle

1. Ouvrir OpenBoard et afficher la bibliothèque **Applications > Outils**.
2. Glisser **Équerre Aristo** sur une page.
3. Vérifier le déplacement, la rotation, les deux poignées de
   redimensionnement et la fermeture.
4. Vérifier que le `0` de la règle reste au milieu après redimensionnement et
   que les nombres croissent vers la gauche comme vers la droite.
5. Vérifier le rapporteur : `0°` à gauche, `90°` au sommet et `180°` à droite;
   les traits sont espacés de 1°, avec traits renforcés tous les 5° et 10°.
6. Tracer au stylet contre chacune des trois arêtes et vérifier que le trait
   reste contraint à l'arête choisie.
7. Tracer sur les guides horizontaux espacés d'un centimètre et vérifier que
   les traits restent parallèles à la base, quelle que soit la rotation de
   l'instrument.
8. Tracer sur l'axe central et vérifier que le trait est exactement
   perpendiculaire à la base. Vérifier également le repère d'angle droit.
9. Commencer un trait dans la couronne du rapporteur aux valeurs 1°, 30°, 45°,
   60°, 89°, 90°, 91°, 135° et 179° et vérifier l'accrochage exact.
10. Vérifier l'échelle verticale et le report d'une même longueur de part et
    d'autre du zéro central.
11. Enregistrer le document, fermer OpenBoard, le rouvrir et vérifier que la
   position, la taille, l'orientation et la rotation sont conservées.
12. Refaire les points 3 à 10 au tactile si l'écran le permet.

## Limitation de validation locale

Le code a été contrôlé statiquement (`git diff --check`) dans l'environnement
de développement fourni. Cet environnement ne contient ni CMake ni Qt, donc
la génération de l'exécutable Windows doit être effectuée sur la machine de
test Windows avec la chaîne de compilation OpenBoard.
