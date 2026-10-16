# Équerre Aristo native pour OpenBoard — cahier des charges V2

## Objectif

Réunir dans un seul instrument natif une règle centrée, une équerre à 45° et
un rapporteur de 0° à 180°. L'instrument doit servir réellement au tracé : les
traits du stylet sont contraints par la géométrie choisie et peuvent être
prolongés au-delà de l'instrument.

## Géométrie et précision

- Triangle rectangle isocèle : côtés inclinés à 45° et angle au sommet à 90°.
- Rapport largeur/hauteur conservé à 2:1 pendant tout redimensionnement.
- Origine commune placée au milieu du bord gradué et utilisée comme centre de
  rotation, zéro de la règle, sommet des angles et origine de la verticale.
- Échelle métrique identique à celle de la règle native OpenBoard, basée sur la
  taille de grille de la scène.
- Calculs des angles en radians à partir d'un degré entier, sans image bitmap
  ni approximation graphique cumulative.

## Mesurer et reporter une longueur

- Zéro exactement au centre de la règle.
- Graduations tous les millimètres, demi-centimètres renforcés et centimètres
  numérotés.
- Valeurs positives identiques et croissantes vers la gauche et vers la
  droite : une longueur peut ainsi être reportée symétriquement autour d'un
  point sans soustraction mentale.
- Graduations recalculées après redimensionnement ; la valeur d'un centimètre
  ne change jamais lorsque l'instrument est agrandi.
- Échelle secondaire le long de l'axe perpendiculaire pour mesurer une hauteur
  ou placer une parallèle à une distance connue.

## Mesurer et tracer un angle

- Arc exact de 0° à 180°, 90° sur l'axe central.
- Graduation à chaque degré, trait renforcé tous les 5° et 10°.
- Valeur numérique tous les 10°.
- Chaque graduation entière constitue un guide de tracé sélectionnable dans
  la couronne du rapporteur : commencer le trait près d'une graduation produit
  une droite contrainte à cet angle exact.
- Les angles 0°, 90° et 180° coïncident exactement avec la base et l'axe
  perpendiculaire.

## Tracer des parallèles et des perpendiculaires

- Bord inférieur utilisable comme règle de tracé.
- Guides horizontaux espacés exactement d'un centimètre, parallèles au bord
  gradué, recalculés selon la taille de l'instrument.
- Axe central exactement perpendiculaire à la base.
- Repère graphique d'angle droit au zéro central.
- Le guide est choisi d'après la distance au segment visible, et non à son
  prolongement infini, afin d'éviter un accrochage erroné.
- Après sélection, la droite peut être prolongée au-delà de l'instrument.

## Comportement natif OpenBoard

- Déplacement à la souris, au stylet et au tactile.
- Rotation autour du zéro central, avec l'accrochage angulaire d'OpenBoard.
- Redimensionnement proportionnel sans déformer les angles.
- Retournement horizontal et vertical.
- Fermeture avec la poignée native.
- Enregistrement et restauration de la position, taille, orientation et
  rotation dans le document OpenBoard.
- Présence dans `Applications > Outils` sous le nom « Équerre Aristo ».

## Recette obligatoire sous Windows

1. Comparer 10 cm de la règle Aristo avec 10 cm de la règle native.
2. Vérifier les deux reports symétriques 3 cm à gauche et 3 cm à droite.
3. Tracer cinq parallèles successives et vérifier leur parallélisme.
4. Tracer une perpendiculaire après rotations de 0°, 17°, 45° et 93°.
5. Tracer les angles 1°, 30°, 45°, 60°, 89°, 90°, 91°, 135° et 179°.
6. Vérifier que les mesures restent identiques après agrandissement et
   réduction de l'instrument.
7. Enregistrer, fermer, rouvrir et contrôler la restauration complète.
8. Refaire les essais essentiels au tactile si l'écran le permet.

## Limite de validation

Les contrôles de cohérence du code et de l'archive peuvent être effectués dans
l'environnement de développement. La validation finale de l'ergonomie et de
la précision au pixel doit être réalisée avec la version compilée d'OpenBoard
sous Windows.
