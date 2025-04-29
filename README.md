# Network Engine
## Description
Ce projet inclut un moteur de jeu fait maison (en C++), un petit jeu de destruction de cubes, et une partie réseau permettant de jouer contre d'autres joueurs. Il a été créé par Albin Horlaville et Hugo Girard dans le cadre du cours de programmation réseau dans les jeux vidéo de l'UQAC à la session d'hiver 2025. Les outils externes utilisés par le projet sont :  
- Magnum et Bullet (pour le moteur de jeu).
- ENet6 (pour la partie réseau).
- Redis et Httplib (pour la partie online).

## Comment lancer le projet ?
Ce projet possède des releases pour Windows, Linux et MacOS. Ces versions sont rangées dans les dossiers **release_windows**, **release_linux** et **release_macOS**. Pour lancer rapidement la partie online du projet, il faut avoir installé Docker.
Afin de lancer le projet il faut :  
1.  Cloner le projet dans le dossier de votre choix.
2.  Ouvrir un terminal Docker, et executer cette commande `docker run --name redis-server -p 6379:6379 -d redis` pour lancer une redis. Tant que cette redis est active, elle sera utilisée à chaque nouveau lancement du jeu.
3.  Ouvrir l'emplacement de clonage le projet, et ouvrir le dossier de release correspondant à votre système d'exploitation.
4.  Aller dans le dossier **GameApi_VotreOS/**, et lancer l'executable `GameApi.exe` (pour Windows) ou executer cette commande `./GameApi` dans le terminal (pour Linux et MacOS).
5.  Aller dans le dossier **ServeurClient_VotreOS/bin/** et lancer l'executable `Serveur.exe` (pour Windows) ou executer cette commande `./Serveur` dans le terminal (pour Linux et MacOS).
6.  Lancer au moins 4 fois l'executable `Client.exe` (pour Windows) ou executer 4 fois cette commande `./Client` dans le terminal (pour Linux et MacOS).
7.  Profiter de ce super jeu !

## Attention
L'exécutable de GameApi pour mac ne fonctionne pas pour une raison que nous n'avons pas eu le temps d'élucider. La version Linux n'a pas pu être testée, car nous ne disposons que de Windows et de Mac. Nous nous excusons pour la gène occasionnée.
