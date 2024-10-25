#include "partyprep.h"
#include "../game.h"

PartyPrep::PartyPrep(Game *game) : GameState(game) {
    send_host_party_settings();
}

void PartyPrep::finished() {
    game->nextGameState<decltype(*this)>();
}

void PartyPrep::process_input(const Message* payload, Client* from) {
    switch(payload->type()) {
        case MessageType::MessageType_PartyPrep: {
            auto partyPrepPayload = payload->payload_as_PartyPrepPayloadType();
            switch(partyPrepPayload->partypreptype()) {
                case PartyPrepType_PartyPrepHostStartGame: {
                    auto input = partyPrepPayload->partypreppayload_as_PartyPrepHostStartGamePayload();
                    if (input->start_game()) {
                        std::cout << "Starting game" << std::endl;
                        finished();
                    }
                    break;
                }
                case PartyPrepType_PartyPrepSettingsRounds: {
                    auto input = partyPrepPayload->partypreppayload_as_PartyPrepSettingsRoundsPayload();
                    game->party->settings->SetNumberOfRounds(input->new_number_of_rounds());
                    send_host_party_settings();
                    break;
                }
                case PartyPrepType_PartyPrepSettingsMiniGame: {
                    auto input = partyPrepPayload->partypreppayload_as_PartyPrepSettingsMiniGamePayload();
                    game->party->settings->ToggleMiniGame(input->name()->str(), input->enable());
                    send_host_party_settings();
                    break;
                }
            }
            break;
        }
    }
}

void PartyPrep::send_host_information() {
    flatbuffers::FlatBufferBuilder builder;
    std::vector<flatbuffers::Offset<FBPlayer>> players_buffer;

    for (Client* client: game->get_clients()) {
        if (client->party->host == client) {
            continue;
        }
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

void PartyPrep::send_host_party_settings() {
    flatbuffers::FlatBufferBuilder builder;
    std::vector<flatbuffers::Offset<FBMiniGameSetting>> minigames_setting_buffer;

    for (MiniGameSettings* setting: game->party->settings->minigames) {
        if (setting != nullptr) {
            auto name = builder.CreateString(setting->name.c_str());
            minigames_setting_buffer.push_back(CreateFBMiniGameSetting(builder, name, setting->enabled));
        }
    }

    auto entities_vector = builder.CreateVector(minigames_setting_buffer);

    // Encode payload to binary
    auto payload = CreatePartyPrepSettingsInformationPayload(builder, game->party->settings->number_of_rounds, entities_vector);

    auto partyPrepPayload = CreatePartyPrepPayloadType(builder, PartyPrepType_PartyPrepSettingsInformation,
                                                      PartyPrepPayload_PartyPrepSettingsInformationPayload, payload.Union());

    // Send payload to host
    game->party->send_party_prep([](Client* client) { return client->party->host == client; }, builder, partyPrepPayload.Union());
}

void PartyPrep::update(int delta_time) {
     send_host_information();
     for (Client* client: game->get_clients()) {
         send_player_information(client->client_id);
     }
}

void PartyPrep::clients_changed(int client_id, bool joined) {
    update(0);
}
