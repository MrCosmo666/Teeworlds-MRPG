#pragma once
#include <tuple>
#include <memory>
#include "user.h"
#include "attachment.h"
#include "embed.h"
#include "permissions.h"
#include "webhook.h"
#include "discord_object_interface.h"
#include "snowflake.h"
#include "channel.h"

// <--- means to add later

namespace SleepyDiscord {
	//declear here since message.h is need for slash commands and this enum is needed for messages
	enum class InteractionType : int {
		NONE = 0, //made up type
		Ping = 1,
		ApplicationCommand = 2,
		MessageComponent = 3,
	};

	struct Emoji : public IdentifiableDiscordObject<Emoji> {
	public:
		~Emoji();
		Emoji() = default;
		//Emoji(const std::string* rawJson);
		Emoji(const json::Value & rawJSON);
		Emoji(const nonstd::string_view& json);
		//Emoji(const json::Values values);
		std::string name;
		std::vector<Snowflake<Role>> roles;
		User user;	//optional
		bool requireColons = false;
		bool managed = false;

		//const static std::initializer_list<const char*const> fields;
		JSONStructStart
			std::make_tuple(
				json::pair                           (&Emoji::ID           , "id"            , json::NULLABLE_FIELD),
				json::pair                           (&Emoji::name         , "name"          , json::NULLABLE_FIELD),
				json::pair<json::ContainerTypeHelper>(&Emoji::roles        , "roles"         , json::OPTIONAL_FIELD),
				json::pair                           (&Emoji::user         , "user"          , json::OPTIONAL_FIELD),
				json::pair                           (&Emoji::requireColons, "require_colons", json::OPTIONAL_FIELD),
				json::pair                           (&Emoji::managed      , "managed"       , json::OPTIONAL_FIELD)
			);
		JSONStructEnd
	};

	struct Reaction : public DiscordObject {
	public:
		Reaction() = default;
		~Reaction();
		//Reaction(const std::string * rawJson);
		Reaction(const json::Value & rawJSON);
		Reaction(const nonstd::string_view & json);
		//Reaction(const json::Values values);
		int count = 0;
		bool me = false;
		Emoji emoji;

		//const static std::initializer_list<const char*const> fields;
		JSONStructStart
			std::make_tuple(
				json::pair(&Reaction::count, "count", json::REQUIRIED_FIELD),
				json::pair(&Reaction::me   , "me"   , json::REQUIRIED_FIELD),
				json::pair(&Reaction::emoji, "emoji", json::REQUIRIED_FIELD)
			);
		JSONStructEnd
	};

	enum class ComponentType {
		NONE = 0, //made up by the library
		ActionRow = 1,
		Button = 2,
	};

	struct BaseComponent : public DiscordObject {
		using Type = ComponentType;
		BaseComponent() = delete;
		BaseComponent(Type _type) : type(_type) {}
		~BaseComponent() = default;

		inline ComponentType getType() const { return type; }

	protected:
		Type type;
	};

	struct RawComponent : public BaseComponent {
		using JSONTypeHelper = json::ClassTypeHelper<json::Value>;

		RawComponent(BaseComponent base, json::Value& rawJSON) : BaseComponent(std::move(base)) {
			data = JSONTypeHelper::toType(rawJSON);
		}
		RawComponent(json::Value& rawJSON) : RawComponent(BaseComponent{static_cast<ComponentType>(rawJSON["type"].GetInt())}, rawJSON) {}
		RawComponent(const nonstd::string_view& json);
		RawComponent(const RawComponent& origin) : BaseComponent(origin.type), data(json::copy(origin.data)) {}

		inline json::Value serialize(typename json::Value::AllocatorType& alloc) const {
			return JSONTypeHelper::fromType(data, alloc);
		}

		inline bool empty() const { return JSONTypeHelper::empty(data); }

		static inline bool isType(const typename json::Value& value) {
			return value.IsObject() && value.FindMember("type") != value.MemberEnd();
		}

		json::Value data;
	};

	template<class Child>
	struct ComponentTemp : public BaseComponent {
		ComponentTemp() : BaseComponent(Child::componentType) {}
		~ComponentTemp() = default;

		inline operator json::Value() {
			return json::toJSON(static_cast<Child&>(*this));
		}

		inline operator RawComponent() {
			return RawComponent{ Child::componentType, operator json::Value() };
		}

		JSONStructStart
			std::make_tuple(
				json::pair<json::EnumTypeHelper>(&Child::type, "type", json::REQUIRIED_FIELD)
			);
		JSONStructEnd
	};

	struct ActionRow : public ComponentTemp<ActionRow> {
		ActionRow() = default;
		~ActionRow() = default;
		ActionRow(json::Value& json);
		ActionRow(const nonstd::string_view& json);
		static const ComponentType componentType = ComponentType::ActionRow;

		std::vector<std::shared_ptr<BaseComponent>> components;

		JSONStructStart
			std::tuple_cat(
				ComponentTemp<ActionRow>::JSONStruct,
				std::make_tuple(
					json::pair<json::ContainerTypeHelper>(&ActionRow::components, "components", json::OPTIONAL_FIELD)
				)
			);
		JSONStructEnd
	};

	enum class ButtonStyle {
		NONE = 0,
		Primary = 1,
		Secondary = 2,
		Success = 3,
		Danger = 4,
		Link = 5,
		DefaultStyle = NONE, //made up for the library to handle null style
	};

	struct Button : public ComponentTemp<Button> {
		Button() = default;
		~Button() = default;
		Button(const json::Value& json);
		Button(const nonstd::string_view& json);
		static const ComponentType componentType = ComponentType::Button;

		ButtonStyle style;
		std::string label;
		Emoji emoji;
		std::string customID;
		std::string url;
		bool disabled = false;

		JSONStructStart
			std::tuple_cat(
				ComponentTemp<Button>::JSONStruct,
				std::make_tuple(
					json::pair<json::EnumTypeHelper>(&Button::style   , "style"    , json::REQUIRIED_FIELD),
					json::pair                      (&Button::label   , "label"    , json::OPTIONAL_FIELD ),
					json::pair                      (&Button::emoji   , "emoji"    , json::OPTIONAL_FIELD ),
					json::pair                      (&Button::customID, "custom_id", json::OPTIONAL_FIELD ),
					json::pair                      (&Button::url     , "url"      , json::OPTIONAL_FIELD ),
					json::pair                      (&Button::disabled, "disabled" , json::OPTIONAL_FIELD )
				)
			);
		JSONStructEnd
	};

	namespace json {
		template<>
		struct ClassTypeHelper<std::shared_ptr<BaseComponent>> {
			using Base = BaseComponent;
			using Type = std::shared_ptr<BaseComponent>;

			static inline Type toType(json::Value& value) {
				const ComponentType type = static_cast<ComponentType>(value["type"].GetInt());
				switch (type) {
				default:
					return std::make_shared<RawComponent>(Base{ type }, value);
				case ComponentType::ActionRow:
					return std::make_shared<ActionRow>(value);
				case ComponentType::Button:
					return std::make_shared<Button>(value);
				}
			}
			static inline json::Value fromType(const Type& value, json::Value::AllocatorType& allocator) {
				const ComponentType type = value->getType();
				switch (type) {
				default:
					return std::static_pointer_cast<RawComponent>(value)->serialize(allocator);
				case ComponentType::ActionRow:
					return json::ClassTypeHelper<ActionRow>::fromType(*std::static_pointer_cast<ActionRow>(value), allocator);
				case ComponentType::Button:
					return json::ClassTypeHelper<Button>::fromType(*std::static_pointer_cast<Button>(value), allocator);
				}
			}
			static inline bool empty(const Type& value) {
				return json::SmartPtrTypeHelper<std::shared_ptr<BaseComponent>, json::ClassTypeHelper>::empty(value);
			}
			static inline bool isType(const json::Value& value) {
				return value.IsObject() && value.FindMember("type") != value.MemberEnd();
			}
		};
	}

	struct StickerPack : public IdentifiableDiscordObject<StickerPack> {
	public:
		StickerPack() = default;
		~StickerPack();
		StickerPack(const json::Value & json);
		StickerPack(const nonstd::string_view & json);

		JSONStructStart
			std::make_tuple(
				json::pair(&StickerPack::ID, "id", json::REQUIRIED_FIELD)
			);
		JSONStructEnd
	};

	struct Sticker : public IdentifiableDiscordObject<Sticker> {
	public:
		Sticker() = default;
		~Sticker();
		Sticker(const json::Value & json);
		Sticker(const nonstd::string_view & json);
		Snowflake<StickerPack> packID;
		std::string name;
		std::string description;
		std::string tags;
		enum class Type : int {
			NONE = 0,
			PNG = 1,
			APNG = 2,
			LOTTIE = 3
		} format;

		JSONStructStart
			std::make_tuple(
				json::pair                      (&Sticker::ID             , "id"              , json::REQUIRIED_FIELD),
				json::pair                      (&Sticker::name           , "name"            , json::OPTIONAL_FIELD ),
				json::pair                      (&Sticker::description    , "description"     , json::OPTIONAL_FIELD ),
				json::pair                      (&Sticker::tags           , "tags"            , json::OPTIONAL_FIELD ),
				json::pair<json::EnumTypeHelper>(&Sticker::format         , "format_type"     , json::OPTIONAL_FIELD )
			);
		JSONStructEnd
	};

	//forward declearion
	class BaseDiscordClient;
	struct Server;
	struct Message;

	struct MessageReference {
	public:
		MessageReference() = default;
		~MessageReference() = default;
		MessageReference(const json::Value& json);
		MessageReference(const nonstd::string_view& json);
		MessageReference(const Message& message);

		Snowflake<Message> messageID;
		Snowflake<Channel> channelID;
		Snowflake<Server> serverID;

		JSONStructStart
			std::make_tuple(
				json::pair(&MessageReference::messageID, "message_id", json::OPTIONAL_FIELD),
				json::pair(&MessageReference::channelID, "channel_id", json::OPTIONAL_FIELD),
				json::pair(&MessageReference::serverID , "guild_id"  , json::OPTIONAL_FIELD)
			);
		JSONStructEnd

		inline bool empty() const {
			return messageID.empty() && channelID.empty() && serverID.empty();
		}
	};

	struct Message : public IdentifiableDiscordObject<Message> {
	public:
		Message() = default;
		~Message() = default;
		Message(json::Value& json);
		Message(const nonstd::string_view& json);
		bool startsWith(const std::string& test);
		std::size_t length();
		bool isMentioned(Snowflake<User> ID);
		bool isMentioned(User& _user);
		Message send(BaseDiscordClient * client);
		Message reply(BaseDiscordClient * client, std::string message,
			Embed embed = Embed()
		);

		Snowflake<Channel> channelID;
		Snowflake<Server> serverID;
		User author;
		ServerMember member;
		std::string content;
		std::string timestamp;
		std::string editedTimestamp;
		bool tts = false;
		bool mentionEveryone = false;
		std::vector<User> mentions;
		std::vector<Snowflake<User>> mentionRoles;
		std::vector<Attachment> attachments;
		std::vector<Embed> embeds;
		std::vector<Reaction> reactions;
		bool pinned = false;
		Snowflake<Webhook> webhookID;
		enum MessageType {
			DEFAULT                                =  0,
			RECIPIENT_ADD                          =  1,
			RECIPIENT_REMOVE                       =  2,
			CALL                                   =  3,
			CHANNEL_NAME_CHANGE                    =  4,
			CHANNEL_ICON_CHANGE                    =  5,
			CHANNEL_PINNED_MESSAGE                 =  6,
			GUILD_MEMBER_JOIN                      =  7,
			USER_PREMIUM_GUILD_SUBSCRIPTION        =  8,
			USER_PREMIUM_GUILD_SUBSCRIPTION_TIER_1 =  9,
			USER_PREMIUM_GUILD_SUBSCRIPTION_TIER_2 = 10,
			USER_PREMIUM_GUILD_SUBSCRIPTION_TIER_3 = 11,
			CHANNEL_FOLLOW_ADD                     = 12,
			GUILD_DISCOVERY_DISQUALIFIED           = 14,
			GUILD_DISCOVERY_REQUALIFIED            = 15,
			REPLY                                  = 19
		} type = DEFAULT;
		std::vector<Sticker> stickers;
		MessageReference messageReference;
		std::shared_ptr<Message> referencedMessage;
		enum class Flags {
			DEFAULT                = 0,
			CROSSPOSTED            = 1 << 0,
			IS_CROSSPOST           = 1 << 1,
			SUPPRESS_EMBEDS        = 1 << 2,
			SOURCE_MESSAGE_DELETED = 1 << 3,
			URGENT                 = 1 << 4,
			EPHEMERAL              = 64
		} flags = Flags::DEFAULT;
		//interaction
		struct Interaction : public IdentifiableDiscordObject<Interaction> {
		public:
			Interaction() = default;
			~Interaction() = default;
			Interaction(const json::Value& json);
			Interaction(const nonstd::string_view& json);
			InteractionType type = InteractionType::NONE;
			std::string name;
			User user;

			JSONStructStart
				std::make_tuple(
					json::pair                      (&Interaction::ID       , "id"        , json::REQUIRIED_FIELD),
					json::pair<json::EnumTypeHelper>(&Interaction::type     , "type"      , json::REQUIRIED_FIELD),
					json::pair                      (&Interaction::name     , "name"      , json::OPTIONAL_FIELD ),
					json::pair                      (&Interaction::user     , "user"      , json::OPTIONAL_FIELD )
				);
			JSONStructEnd
		};
		Interaction interaction;
		Snowflake<DiscordObject> applicationID;
		//components
		std::vector<std::shared_ptr<BaseComponent>> components;

		//const static std::initializer_list<const char*const> fields;
		JSONStructStart
			std::make_tuple(
				json::pair                           (&Message::ID               , "id"                , json::REQUIRIED_FIELD),
				json::pair                           (&Message::channelID        , "channel_id"        , json::REQUIRIED_FIELD),
				json::pair                           (&Message::serverID         , "guild_id"          , json::OPTIONAL_FIELD ),
				json::pair                           (&Message::author           , "author"            , json::REQUIRIED_FIELD),
				json::pair                           (&Message::content          , "content"           , json::REQUIRIED_FIELD),
				json::pair                           (&Message::member           , "member"            , json::OPTIONAL_FIELD ),
				json::pair                           (&Message::timestamp        , "timestamp"         , json::REQUIRIED_FIELD),
				json::pair                           (&Message::editedTimestamp  , "edited_timestamp"  , json::NULLABLE_FIELD ),
				json::pair                           (&Message::tts              , "tts"               , json::REQUIRIED_FIELD),
				json::pair                           (&Message::mentionEveryone  , "mention_everyone"  , json::REQUIRIED_FIELD),
				json::pair<json::ContainerTypeHelper>(&Message::mentions         , "mentions"          , json::REQUIRIED_FIELD),
				json::pair<json::ContainerTypeHelper>(&Message::mentionRoles     , "mention_roles"     , json::REQUIRIED_FIELD),
				json::pair<json::ContainerTypeHelper>(&Message::attachments      , "attachments"       , json::REQUIRIED_FIELD),
				json::pair<json::ContainerTypeHelper>(&Message::embeds           , "embeds"            , json::REQUIRIED_FIELD),
				json::pair<json::ContainerTypeHelper>(&Message::reactions        , "reactions"         , json::OPTIONAL_FIELD ),
				json::pair                           (&Message::pinned           , "pinned"            , json::REQUIRIED_FIELD),
				json::pair                           (&Message::webhookID        , "webhook_id"        , json::OPTIONAL_FIELD ),
				json::pair<json::EnumTypeHelper     >(&Message::type             , "type"              , json::REQUIRIED_FIELD),
				json::pair<json::ContainerTypeHelper>(&Message::stickers         , "stickers"          , json::OPTIONAL_FIELD ),
				json::pair                           (&Message::messageReference , "message_reference" , json::OPTIONAL_FIELD ),
				json::pair<json::SmartPtrTypeHelper >(&Message::referencedMessage, "referenced_message", json::OPTIONAL_FIELD ),
				json::pair<json::EnumTypeHelper     >(&Message::flags            , "flags"             , json::OPTIONAL_FIELD ),
				json::pair                           (&Message::interaction      , "interaction"       , json::OPTIONAL_FIELD ),
				json::pair                           (&Message::applicationID    , "application_id"    , json::OPTIONAL_FIELD ),
				json::pair<json::ContainerTypeHelper>(&Message::components       , "components"        , json::OPTIONAL_FIELD )
			);
		JSONStructEnd
	};

	inline MessageReference::MessageReference(const Message& message) :
		messageID(message.ID),
		channelID(message.channelID),
		serverID(message.serverID)
	{}

	struct MessageRevisions {
		MessageRevisions(json::Value& json) :
			messageID(json["id"]), channelID(json["channel_id"]), RevisionsJSON(json)
		{}
		inline void applyChanges(Message& outOfDateMessage) {
			assert(outOfDateMessage.ID == messageID);
			json::fromJSON(outOfDateMessage, RevisionsJSON);
		}
		Snowflake<Message> messageID;
		Snowflake<Channel> channelID;
		json::Value& RevisionsJSON;
	};

	enum class MentionReplierFlag : char {
		NotSet = -2,
		DoNotMentionReply = false,
		MentionReply = true
	};

	//allow mentions parse has different behaviors when undefined and empty.
	//allow mentions parse is also a array. the other values that have this
	//kind of behavior, where a value makes discord do different thinsg based
	//on it being defined or not, is that they are a primitive json type or
	//an object. This one is an array, making it special.
	template<class Container, template<class...> class TypeHelper>
	struct AllowMentionsParseHelper :
		public json::ToContainerFunction<Container, TypeHelper>,
		public json::FromContainerFunction<Container, TypeHelper>,
		public json::IsArrayFunction
	{
		static inline bool empty(const Container& value) {
			return value.size() == 1 && value.front().empty();
		}
	};

	struct AllowedMentions {
	public:
		using ParseValueType = std::string;
		using ParseContainer = std::vector<std::string>;

		AllowedMentions() = default;
		~AllowedMentions() = default;
		AllowedMentions(int) : parse({}) {}
		AllowedMentions(const json::Value & json);
		AllowedMentions(const nonstd::string_view & json);
		ParseContainer parse = {""};
		std::vector<Snowflake<Role>> roles;
		std::vector<Snowflake<User>> users;
		enum class MentionReplierFlag : char {
			NotSet = -2,
			WillNotMentionReply = false,
			MentionReply = true
		};
		MentionReplierFlag repliedUser = MentionReplierFlag::NotSet;

		JSONStructStart
			std::make_tuple(
				json::pair<json::ContainerTypeHelper>(&AllowedMentions::parse      , "parse"       , json::OPTIONAL_FIELD),
				json::pair<json::ContainerTypeHelper>(&AllowedMentions::roles      , "roles"       , json::OPTIONAL_FIELD),
				json::pair<json::ContainerTypeHelper>(&AllowedMentions::users      , "users"       , json::OPTIONAL_FIELD),
				json::pair<json::EnumTypeHelper     >(&AllowedMentions::repliedUser, "replied_user", json::OPTIONAL_FIELD)
			);
		JSONStructEnd

		inline bool empty() const {
			return (parse.size() == 1 && parse[0].empty()) && repliedUser == MentionReplierFlag::NotSet;
		}
	};

	template<>
	struct GetDefault<AllowedMentions::MentionReplierFlag> {
		static inline AllowedMentions::MentionReplierFlag get() {
			return AllowedMentions::MentionReplierFlag::NotSet;
		}
	};

	template<>
	struct GetEnumBaseType<AllowedMentions::MentionReplierFlag> {
		//this makes the json wrapper know to use getBool instead of getInt
		using Value = bool;
	};

	template<class Type>
	struct MessageParams : public DiscordObject {
		Snowflake<Channel> channelID;
		std::string content = {};
		Embed embed = Embed::Flag::INVALID_EMBED;
		AllowedMentions allowedMentions;
		std::vector<std::shared_ptr<BaseComponent>> components;

		JSONStructStart
			std::make_tuple(
				json::pair                           (&Type::content        , "content"         , json::OPTIONAL_FIELD),
				json::pair                           (&Type::embed          , "embed"           , json::OPTIONAL_FIELD),
				json::pair                           (&Type::allowedMentions, "allowed_mentions", json::OPTIONAL_FIELD),
				json::pair<json::ContainerTypeHelper>(&Type::components     , "components"      , json::OPTIONAL_FIELD)
			);
		JSONStructEnd
	};

	struct SendMessageParams : MessageParams<SendMessageParams> {
	public:
		bool tts = false;
		MessageReference messageReference;

		JSONStructStart
			std::tuple_cat(
				MessageParams<SendMessageParams>::JSONStruct,
				std::make_tuple(
					json::pair(&SendMessageParams::tts             , "tts"              , json::OPTIONAL_FIELD ),
					json::pair(&SendMessageParams::messageReference, "message_reference", json::OPTIONAL_FIELD )
				)
			);
		JSONStructEnd
	};

	struct EditMessageParams : MessageParams<EditMessageParams> {
	public:
		Snowflake<Message> messageID;
		Message::Flags flags = Message::Flags::DEFAULT;
		std::vector<Attachment> attachments;

		JSONStructStart
			std::tuple_cat(
				MessageParams<EditMessageParams>::JSONStruct,
				std::make_tuple(
					json::pair<json::EnumTypeHelper     >(&EditMessageParams::flags      , "flags"      , json::OPTIONAL_FIELD),
					json::pair<json::ContainerTypeHelper>(&EditMessageParams::attachments, "attachments", json::OPTIONAL_FIELD)
				)
			);
		JSONStructEnd
	};

	struct EditWebhookParams : public DiscordObject {
	public:
		std::string content;
		std::vector<Embed> embeds;
		AllowedMentions allowedMentions;
		std::vector<std::shared_ptr<BaseComponent>> components;
		JSONStructStart
			std::make_tuple(
				json::pair                           (&EditWebhookParams::content        , "content"         , json::OPTIONAL_FIELD),
				json::pair<json::ContainerTypeHelper>(&EditWebhookParams::embeds         , "embeds"          , json::OPTIONAL_FIELD),
				json::pair                           (&EditWebhookParams::allowedMentions, "allowed_mentions", json::OPTIONAL_FIELD),
				json::pair<json::ContainerTypeHelper>(&EditWebhookParams::components     , "components"      , json::OPTIONAL_FIELD)
			);
		JSONStructEnd
	};

	struct WebHookParams : public EditWebhookParams {
	public:
		std::string username;
		std::string avatarURL;
		bool tts = false;
		JSONStructStart
			std::make_tuple(
				json::pair                           (&WebHookParams::content         , "content"          , json::OPTIONAL_FIELD),
				json::pair                           (&WebHookParams::username        , "username"         , json::OPTIONAL_FIELD),
				json::pair                           (&WebHookParams::avatarURL       , "avatar_url"       , json::OPTIONAL_FIELD),
				json::pair                           (&WebHookParams::tts             , "tts"              , json::OPTIONAL_FIELD),
				json::pair<json::ContainerTypeHelper>(&WebHookParams::embeds          , "embeds"           , json::OPTIONAL_FIELD),
				json::pair                           (&WebHookParams::allowedMentions , "allowed_mentions" , json::OPTIONAL_FIELD),
				json::pair<json::ContainerTypeHelper>(&WebHookParams::components      , "components"       , json::OPTIONAL_FIELD)
			);
		JSONStructEnd
	};
}