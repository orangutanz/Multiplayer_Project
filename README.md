# Multiplayer_Project

This is a project for testing out Unreal Engine's multiplayer framework features.
Developed with Unreal Engine 5

In this project:
1.Start the game as a singleplayer.
2.Host/search/join a multiplayer game.
3.Uses ReplicationGraph. (For advance replication control. Designed for deticated server, might not work proper with the host player) 
    -GridSpatialization2D  >> Divide replications into groups base on position
    -ReplicationGraphNode_Instance  >> Instance based, player in different instance can't see each other.

