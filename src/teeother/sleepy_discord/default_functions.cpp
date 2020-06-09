#include "client.h"

namespace SleepyDiscord {
	void BaseDiscordClient::onReady(std::string* jsonMessage) { return; }
	void BaseDiscordClient::onResumed(std::string * jsonMessage) { return; }
	void BaseDiscordClient::onDeleteServer(std::string * jsonMessage) { return; }
	void BaseDiscordClient::onEditServer(std::string * jsonMessage)  { return; } 
	void BaseDiscordClient::onBan(std::string * jsonMessage)  { return; } 
	void BaseDiscordClient::onUnban(std::string * jsonMessage)  { return; } 
	void BaseDiscordClient::onMember(std::string * jsonMessage)  { return; } 
	void BaseDiscordClient::onRemoveMember(std::string * jsonMessage)  { return; } 
	void BaseDiscordClient::onDeleteMember(std::string * jsonMessage)  { return; } 
	void BaseDiscordClient::onEditMember(std::string * jsonMessage)  { return; } 
	void BaseDiscordClient::onRole(std::string * jsonMessage)  { return; } 
	void BaseDiscordClient::onDeleteRole(std::string * jsonMessage)  { return; } 
	void BaseDiscordClient::onEditRole(std::string * jsonMessage)  { return; } 
	void BaseDiscordClient::onEditEmojis(std::string * jsonMessage)  { return; } 
	void BaseDiscordClient::onMemberChunk(std::string * jsonMessage)  { return; } 
	void BaseDiscordClient::onDeleteChannel(std::string * jsonMessage)  { return; } 
	void BaseDiscordClient::onEditChannel(std::string * jsonMessage)  { return; } 
	void BaseDiscordClient::onPinMessage(std::string * jsonMessage)  { return; } 
	void BaseDiscordClient::onPresenceUpdate(std::string * jsonMessage)  { return; } 
	void BaseDiscordClient::onEditUser(std::string * jsonMessage)  { return; } 
	void BaseDiscordClient::onEditUserNote(std::string * jsonMessage)  { return; } 
	void BaseDiscordClient::onEditUserSettings(std::string * jsonMessage)  { return; }
	void BaseDiscordClient::onEditVoiceState(std::string * jsonMessage)  { return; } 
	void BaseDiscordClient::onTyping(std::string * jsonMessage)  { return; } 
	void BaseDiscordClient::onDeleteMessage(std::string * jsonMessage)  { return; }
	void BaseDiscordClient::onEditMessage(std::string * jsonMessage)  { return; } 
	void BaseDiscordClient::onBulkDelete(std::string * jsonMessage)  { return; } 
	void BaseDiscordClient::onEditVoiceServer(std::string * jsonMessage)  { return; } 
	void BaseDiscordClient::onServerSync(std::string * jsonMessage)  { return; }
	void BaseDiscordClient::onRelationship(std::string * jsonMessage)  { return; } 
	void BaseDiscordClient::onRemoveRelationship(std::string * jsonMessage)  { return; }
	void BaseDiscordClient::onDeleteRelationship(std::string * jsonMessage)  { return; }
	void BaseDiscordClient::onReaction(std::string * jsonMessage)  { return; }
	void BaseDiscordClient::onRemoveReaction(std::string * jsonMessage)  { return; }
	void BaseDiscordClient::onDeleteReaction(std::string * jsonMessage)  { return; }
	void BaseDiscordClient::onRemoveAllReaction(std::string * jsonMessage)  { return; }
	void BaseDiscordClient::onDeleteAllReaction(std::string * jsonMessage)  { return; }
	void BaseDiscordClient::onMessage(Message message)  { return; }
	void BaseDiscordClient::onEditedMessage(std::string* jsonMessage)  { return; }
	void BaseDiscordClient::onHeartbeat()  { return; }
	void BaseDiscordClient::onHeartbeatAck()  { return; } 
	void BaseDiscordClient::onServer(Server jsonMessage)  { return; } 
	void BaseDiscordClient::onChannel(std::string* jsonMessage)  { return; } 
	void BaseDiscordClient::onEditedRole(std::string* jsonMessage)  { return; } 
	void BaseDiscordClient::onDispatch(std::string * jsonMessage)  { return; } 
	void BaseDiscordClient::onInvaldSession()  { return; } 
	void BaseDiscordClient::onDisconnect()  { return; } 
	void BaseDiscordClient::onResume()  { return; } 
	void BaseDiscordClient::runAsync()  { return; } 
	void BaseDiscordClient::run()  { return; } 
	void BaseDiscordClient::onQuit()  { return; } 
	void SleepyDiscord::BaseDiscordClient::onResponse(Response response)  { return; } 
	void BaseDiscordClient::sleep(const unsigned int milliseconds)  { return; } 
	void BaseDiscordClient::fileRead(const char* path, std::string*const file)  { return; } 
	void BaseDiscordClient::tick(float deltaTime)  { return; } 
	void BaseDiscordClient::onError(ErrorCode errorCode, std::string errorMessage)  { return; } 
}