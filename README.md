# TP server chat

## Specifications

Ce serveur de chat permet la communication entre client et serveur dans les deux sens.
Il est possible de connecter plusieurs clients.
Lorsqu'un client se connecte, le serveur lui demande un nom pour l'identifier.

### Commandes

`quit` : la commande quit permet de fermer le serveur de chat

`send` : la commande send permet de contrôler la destination des messages. Voici les options
   * `[nom du client]` : le nom du client pour cibler l'envoi du message
   * `all` : Pour envoyer à tous les clients

=> Exemple : `send Client_1 test client 1` enverra 'test client 1' au Client 1

=> Exemple : `send all test tous clients` enverra 'test tous clients' à tous les clients

`` : sans commande particulière, le message entré sera envoyé à tous les clients

=> Exemple : `test tous clients` enverra 'test tous clients' à tous les clients

## Lancement du serveur

1. Build le serveur
    ```bash
    make clean et make
    ```
2. Lancer le serveur
    ```bash
    ./chat.bin
    ```