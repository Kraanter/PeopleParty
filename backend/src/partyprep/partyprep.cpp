#include "partyprep.h"
#include "../game.h"

PartyPrep::PartyPrep(Game *game) : GameState(game) {

}

void PartyPrep::finished() {
    game->nextGameState<decltype(*this)>();
}

void PartyPrep::process_partyprep_input(const PartyPrepPayloadType *payload, Client *from)
{
    switch(payload->partypreptype()) {
        case PartyPrepType_PartyPrepHostStartGame: {
            auto input = payload->partypreppayload_as_PartyPrepHostStartGamePayload();
            if (input->start_game()) {
                finished();
            }
        }
    }
}

void PartyPrep::send_host_information() {
    flatbuffers::FlatBufferBuilder builder;
    std::vector<flatbuffers::Offset<FBPlayer>> players_buffer;

    for (Client* client: game->get_clients()) {
        auto name = builder.CreateString(client->name.c_str());
        players_buffer.push_back(CreateFBPlayer(builder, name));
    }
    auto entities_vector = builder.CreateVector(players_buffer);

    // Encode payload to binary
    auto payload = CreatePartyPrepHostInformationPayload(builder, game->getPartyId(), entities_vector);

    auto partyPrepPayload = CreatePartyPrepPayloadType(builder, PartyPrepType_PartyPrepHostInformation,
                                                      PartyPrepPayload_PartyPrepHostInformationPayload, payload.Union());

    // Send payload to host
    game->party->send_party_prep([](Client* client) { return client->party->host == client; }, builder, partyPrepPayload.Union());
}

void PartyPrep::send_player_information(int client_id) {
    std::cout << "Sending player information" << std::endl;
    flatbuffers::FlatBufferBuilder builder;
    
    auto payload = CreatePartyPrepPlayerInformationPayload(builder, game->getPartyId());
    
    auto partyPrepPayload = CreatePartyPrepPayloadType(builder, PartyPrepType_PartyPrepPlayerInformation,
                                                      PartyPrepPayload_PartyPrepPlayerInformationPayload, payload.Union());

    // Send payload to host
    game->party->send_party_prep([client_id](Client* client) { return client->client_id == client_id; }, builder, partyPrepPayload.Union());
}

void PartyPrep::update(int delta_time) {
     send_host_information();
     for (Client* client: game->get_clients()) {
         send_player_information(client->client_id);
     }
}

void PartyPrep::clients_changed() {
    std::cout << "players changed" << std::endl;
    update(0);
}
