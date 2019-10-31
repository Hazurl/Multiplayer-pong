# Protocol

* [New user](#new-user)
    * [Invalid](#invalid)
    * [Connecting](#connecting)
* [Lobby](#lobby)
    * [New](#new)
    * [Regular User](#regular-user)
    * [Entering Room](#entering-room)
    * [Creating Room](#creating-room)
* [Room](#room)
    * [New](#new-1)
    * [Leaving](#leaving)
    * [Spectator](#spectator)
    * [Queued](#queued)
    * [AcceptingBePlayer](#acceptingbeplayer)
    * [NextPlayer](#nextplayer)
    * [Player](#player)

Any packets not listed in the states will result in the server dropping the connection or ignoring the packet.

## New user

### Invalid

When a client just got connected to the server or when its usernae didn't got approved.

| Sender | Packet | Next state |
|--------|--------|------------|
| Client | **`client::ChangeUsername`** | [**`NewUser::Connecting`**](#Connecting) |

### Connecting

When a client is waiting for the approval of its username.

| Sender | Packet | Next state |
|--------|--------|------------|
| Server | Valid **`server::ChangeUsernameResponse`** | [**`Lobby::New`**](#New) |
| Server | Invalid **`server::ChangeUsernameResponse`** | [**`NewUser::Invalid`**](#Invalid) |


## Lobby

When a client is connected under a username.

### New

When a client is waiting for the information about the lobby.

| Sender | Packet | Next state |
|--------|--------|------------|
| Server | **`server::LobbyInfo`** | [**`Lobby::RegularUser`**](#Regular-User) |

### Regular User

When a client is chilling in the lobby and can contemplate the users and rooms.

| Sender | Packet | Next state |
|--------|--------|------------|
| Server | **`server::NewUser`** | |
| Server | **`server::OldUser`** | |
| Server | **`server::NewRoom`** | |
| Server | **`server::OldRoom`** | |
| Server | **`server::RoomInfo`** | |
| Client | **`client::SubscribeRoomInfo`** | |
| Client | **`client::EnterRoom`** | [**`Lobby::EnteringRoom`**](#Entering-Room) |
| Client | **`client::CreateRoom`** | [**`Lobby::CreatingRoom`**](#Creating-Room) |

### Entering Room

When the client attempt to enter a room.

| Sender | Packet | Next state |
|--------|--------|------------|
| Server | **`server::NewUser`** | |
| Server | **`server::OldUser`** | |
| Server | **`server::NewRoom`** | |
| Server | **`server::OldRoom`** | |
| Server | **`server::RoomInfo`** | |
| Server | Valid **`server::EnterRoomResponse`** | [**`Room::New`**](#New-1) |
| Server | Invalid **`server::EnterRoomResponse`** | [**`Lobby::RegularUser`**](#Regular-User) |

### Creating Room

When the client attempt to create an empty room.

| Sender | Packet | Next state |
|--------|--------|------------|
| Server | **`server::NewUser`** | |
| Server | **`server::OldUser`** | |
| Server | **`server::NewRoom`** | |
| Server | **`server::OldRoom`** | |
| Server | **`server::RoomInfo`** | |
| Server | Valid **`server::CreateRoomResponse`** | [**`Room::New`**](#New-1) |
| Server | Invalid **`server::CreateRoomResponse`** | [**`Lobby::RegularUser`**](#Regular-User) |

## Room

When a client just joined a room.

### New

When a client is waiting for the information about the room.

| Sender | Packet | Next state |
|--------|--------|------------|
| Server | **`server::RoomInfo`** | [**`Room::Spectator`**](#Spectator) |

### Leaving

When a client try to leave the room.

| Sender | Packet | Next state |
|--------|--------|------------|
| Server | **`server::NewUser`** | |
| Server | **`server::OldUser`** | |
| Server | **`server::NewPlayer`** | |
| Server | **`server::OldPlayer`** | |
| Server | **`server::Score`** | |
| Server | **`server::GameState`** | |
| Server | **`server::GameOver`** | |
| Server | Valid **`server::LeaveRoomResponse`** | [**`Lobby::New`**](#New) |
| Server | Invalid **`server::LeaveRoomResponse`** | [**`Room::Spectator`**](#Spectator) |

> **Ignored packets:**
>
> | Sender | Packet |
> |--------|--------|
> | Server | **`server::DeniedBePlayer`** | 
> | Server | **`server::BeNextPlayer`** | 

### Spectator

When a client is watching the current game being played (if there's one).

| Sender | Packet | Next state |
|--------|--------|------------|
| Server | **`server::NewUser`** | |
| Server | **`server::OldUser`** | |
| Server | **`server::NewPlayer`** | |
| Server | **`server::OldPlayer`** | |
| Server | **`server::Score`** | |
| Server | **`server::GameState`** | |
| Server | **`server::GameOver`** | |
| Client | **`client::EnterQueue`** | [**`Room::Queued`**](#Queued) |
| Client | **`client::LeaveRoom`** | [**`Room::Leaving`**](#Leaving) |

> **Ignored packets:**
>
> | Sender | Packet |
> |--------|--------|
> | Server | **`server::DeniedBePlayer`** | 
> | Server | **`server::BeNextPlayer`** | 
> | Client | **`client::AcceptBePlayer`** | 
> | Client | **`client::LeaveQueue`** | 
> | Client | **`client::Input`** | 
> | Client | **`client::Abandon`** | 

### Queued

When a client is waiting to play while still watching the game being played.

| Sender | Packet | Next state |
|--------|--------|------------|
| Server | **`server::NewUser`** | |
| Server | **`server::OldUser`** | |
| Server | **`server::NewPlayer`** | |
| Server | **`server::OldPlayer`** | |
| Server | **`server::Score`** | |
| Server | **`server::GameState`** | |
| Server | **`server::GameOver`** | |
| Server | **`server::BeNextPlayer`** | [**`Room::AcceptingBePlayer`**](#AcceptingBePlayer) |
| Client | **`client::LeaveRoom`** | [**`Room::Leaving`**](#Leaving) |
| Client | **`client::LeaveQueue`** | [**`Room::Spectator`**](#Spectator) |

> **Ignored packets:**
>
> | Sender | Packet |
> |--------|--------|
> | Server | **`server::DeniedBePlayer`** | 
> | Client | **`client::Input`** | 
> | Client | **`client::Abandon`** | 

### AcceptingBePlayer

When a client got accepted to be the next player and the server is waiting for the approval of the client.

| Sender | Packet | Next state |
|--------|--------|------------|
| Server | **`server::NewUser`** | |
| Server | **`server::OldUser`** | |
| Server | **`server::NewPlayer`** | |
| Server | **`server::OldPlayer`** | |
| Server | **`server::Score`** | |
| Server | **`server::GameState`** | |
| Server | **`server::GameOver`** | |
| Server | **`server::DeniedBePlayer`** | [**`Room::Spectator`**](#Spectator) |
| Client | **`client::AcceptBePlayer`** | [**`Room::NextPlayer`**](#NextPlayer) |
| Client | **`client::LeaveRoom`** | [**`Room::Leaving`**](#Leaving) |
| Client | **`client::LeaveQueue`** | [**`Room::Spectator`**](#Spectator) |

> **Ignored packets:**
>
> | Sender | Packet |
> |--------|--------|
> | Client | **`client::Input`** | 
> | Client | **`client::Abandon`** | 

### NextPlayer

When the client approve to play and is waiting for the server to approve its approval.

| Sender | Packet | Next state |
|--------|--------|------------|
| Server | **`server::NewUser`** | |
| Server | **`server::OldUser`** | |
| Server | **`server::NewPlayer`** | |
| Server | **`server::OldPlayer`** | |
| Server | **`server::Score`** | |
| Server | **`server::GameState`** | |
| Server | **`server::GameOver`** | |
| Server | **`server::DeniedBePlayer`** | [**`Room::Spectator`**](#Spectator) |
| Server | **`server::BePlayer`** | [**`Room::Player`**](#Player) |

### Player

When a client is a player.

| Sender | Packet | Next state |
|--------|--------|------------|
| Server | **`server::NewUser`** | |
| Server | **`server::OldUser`** | |
| Server | **`server::NewPlayer`** | |
| Server | **`server::OldPlayer`** | |
| Server | **`server::Score`** | |
| Server | **`server::GameState`** | |
| Server | **`server::GameOver`** | [**`Room::Queued`**](#Queued) |
| Client | **`client::Input`** | |
| Client | **`client::Abandon`** | |
