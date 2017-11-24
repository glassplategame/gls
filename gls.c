#include "gls.h"

// Key for thread-specific buffer.
pthread_key_t gls_key;

// Static functions.
ssize_t gls_rdwrn(int fd, void* buffer, size_t count,
	ssize_t(*rdwr)(int fd, void* buffer, size_t count)) {
	char* buf;
	ssize_t i;
	int ret;

	// Read in 'n' bytes.
	buf = (char*)buffer;
	for (i = 0; i < count; ) {
		ret = rdwr(fd, buf, count);
		if (ret == -1) {
			// Read error.
			return -1;
		} else if (!ret) {
			// EOF.
			return i;
		}
		i += ret;
		buf += ret;
		count -= ret;
	}
	return i;
}

static ssize_t gls_rdwrvn(int fd, struct iovec* iov, int iovcnt,
	ssize_t(*rdwrv)(int fd, const struct iovec* iov, int iovcnt)) {
	int cnt;
	int ret;

	// Read in 'n' bytes.
	cnt = 0;
	do {
		// Actual read.
		ret = rdwrv(fd, iov, iovcnt);
		if (ret == -1) {
			// Error.
			return -1;
		} else if (!ret) {
			// EOF.
			return cnt;
		}
		cnt += ret;

		// Update iovs.
		while (iovcnt && iov[0].iov_len <= ret) {
			ret -= iov[0].iov_len;
			iov++;
			iovcnt--;
		}
		if (ret) {
			iov[0].iov_len -= ret;
		}
	} while (iovcnt);
	return cnt;
}

// Library functions.
void gls_header_marshal(char* buffer, uint32_t event) {
	uint32_t tmp;

	// Write header data.
	tmp = htonl(event);
	memcpy(buffer, &tmp, sizeof(uint32_t));
}

struct flub* gls_header_read(struct gls_header* header, int fd) {
	// Read header.
	if (gls_readn(fd, &header->event, sizeof(uint32_t)) <
		sizeof(uint32_t)) {
		// Failed/partial read.
		return g_flub_toss("Unable to read header: '%s'",
			g_serr(errno));
	}
	header->event = ntohl(header->event);
	return NULL;
}

struct flub* gls_init() {
	char* buffer;
	struct flub* flub;
	static int key_created = 0;
	static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
	int ret;

	// Initialize key.
	flub = NULL;
	ret = pthread_mutex_lock(&mutex);
	if (ret) {
		return g_flub_toss("Unable to lock mutex: '%s'",
			g_serr(ret));
	}
	if (!key_created) {
		ret = pthread_key_create(&gls_key, gls_init_destructor);
		if (ret) {
			flub = g_flub_toss("Unable to create key: '%s'",
				g_serr(ret));
			goto unlock;
		}
		key_created = 1;
	}
unlock:
	ret = pthread_mutex_unlock(&mutex);
	if (ret && flub) {
		flub_append(flub, "Mutex unlock failed: '%s'",
			g_serr(ret));
	} else if (ret) {
		flub = g_flub_toss("Mutex unlock failed: '%s'",
			g_serr(ret));
	}
	if (flub) {
		return flub;
	}

	// Initialize buffer.
	buffer = (char*)malloc(GLS_BUFFER_SIZE);
	if (!buffer) {
		return g_flub_toss("Unable to allocate buffer");
	}

	// Set buffer.
	ret = pthread_setspecific(gls_key, buffer);
	if (ret) {
		return g_flub_toss("Unable to set pthread buffer: '%s'",
			g_serr(ret));
	}
	return NULL;
}

void gls_init_destructor(void* buffer) {
	free(buffer);
}

struct flub* gls_nick_set_read(struct gls_nick_set* set, int fd,
	int validate) {
	struct flub* flub;
	int i;
	struct iovec iovs[2];
	ssize_t len;

	// Read nick set.
	memset(set, 0, sizeof(struct gls_nick_set));
	len = 0;
	iovs[0].iov_base = &set->nick;
	len += iovs[0].iov_len = GLS_NAME_LENGTH;
	iovs[1].iov_base = &set->reason;
	len += iovs[1].iov_len = GLS_NICK_SET_REASON;
	if (gls_readvn(fd, iovs, 2) < len) {
		return g_flub_toss("Unable to read nick set: '%s'",
			g_serr(errno));
	}

	// Validate nick set.
	if (!validate) {
		return NULL;
	}
	flub = gls_nick_validate(set->nick, 1);
	if (flub) {
		return flub_append(flub, "reading nick set request");
	}
	for (i = 0; i < GLS_NICK_SET_REASON - 1; i++) {
		if ((!isprint(set->reason[i])) && (set->reason[i] != '\0')) {
			return g_flub_toss("Invalid character in nick set "
				"failure reason");
		}
	}
	set->reason[GLS_NICK_SET_REASON - 1] = '\0';
	return NULL;
}

struct flub* gls_nick_set_write(struct gls_nick_set* set, int fd) {
	char* buf;
	char* cur;
	int32_t len;

	// Get buffer.
	buf = pthread_getspecific(gls_key);
	if (!buf) {
		return g_flub_toss("Unable to get buffer");
	}

	// Prepare header.
	cur = buf;
	gls_header_marshal(cur, GLS_EVENT_NICK_SET);
	cur += 4;
	len = 4;

	// Prepare nick set.
	memcpy(cur, set->nick, GLS_NAME_LENGTH);
	cur += GLS_NAME_LENGTH;
	len += GLS_NAME_LENGTH;
	memcpy(cur, set->reason, GLS_NICK_SET_REASON);
	cur += GLS_NICK_SET_REASON;
	len += GLS_NICK_SET_REASON;

	// Write packet.
	if (gls_writen(fd, buf, len) < len) {
		return g_flub_toss("Unable to write nick set: '%s'",
			g_serr(errno));
	}
	return NULL;
}


struct flub* gls_nick_change_read(struct gls_nick_change* change, int fd,
	int validate) {
	struct flub* flub;
	struct iovec iovs[2];
	ssize_t len;

	// Read nick change.
	memset(change, 0, sizeof(struct gls_nick_change));
	len = 0;
	iovs[0].iov_base = &change->old;
	len += iovs[0].iov_len = GLS_NAME_LENGTH;
	iovs[1].iov_base = &change->new;
	len += iovs[1].iov_len = GLS_NAME_LENGTH;
	if (gls_readvn(fd, iovs, 2) < len) {
		return g_flub_toss("Unable to read nick change: '%s'",
			g_serr(errno));
	}

	// Validate nick change.
	if (!validate) {
		return NULL;
	}
	flub = gls_nick_validate(change->old, 0);
	if (flub) {
		return flub_append(flub, "validating nick change old nick");
	}
	flub = gls_nick_validate(change->new, 0);
	if (flub) {
		return flub_append(flub, "validating nick change new nick");
	}
	return NULL;
}

struct flub* gls_nick_change_write(struct gls_nick_change* change, int fd) {
	// Write nick change.
	char* buf;
	char* cur;
	ssize_t len;

	// Get buffer.
	buf = pthread_getspecific(gls_key);
	if (!buf) {
		return g_flub_toss("Unable to get buffer");
	}

	// Prepare header.
	cur = buf;
	gls_header_marshal(cur, GLS_EVENT_NICK_CHANGE);
	cur += 4;
	len = 4;

	// Prepare nick change.
	memcpy(cur, change->old, GLS_NAME_LENGTH);
	cur += GLS_NAME_LENGTH;
	len += GLS_NAME_LENGTH;
	memcpy(cur, change->new, GLS_NAME_LENGTH);
	cur += GLS_NAME_LENGTH;
	len += GLS_NAME_LENGTH;

	// Write packet.
	if (gls_writen(fd, buf, len) < len) {
		return g_flub_toss("Unable to write nick change: '%s'",
			g_serr(errno));
	}
	return NULL;
}

struct flub* gls_nick_req_read(struct gls_nick_req* req, int fd, int validate) {
	struct flub* flub;
	ssize_t size = sizeof(req->nick);

	// Read nick request.
	memset(req, 0, sizeof(struct gls_nick_req));
	if (gls_readn(fd, req->nick, size) < size) {
		return g_flub_toss("Unable to read nick request: %s",
			g_serr(errno));
	}

	// Validate nick.
	if (!validate) {
		return NULL;
	}
	if ((flub = gls_nick_validate(req->nick, 0))) {
		return flub_append(flub, "reading nick request");
	}
	return NULL;
}

struct flub* gls_nick_req_write(struct gls_nick_req* req, int fd) {
	char* buf;
	char* cur;
	int32_t len;

	// Get buffer.
	buf = pthread_getspecific(gls_key);
	if (!buf) {
		return g_flub_toss("Unable to get buffer");
	}

	// Prepare header.
	cur = buf;
	gls_header_marshal(cur, GLS_EVENT_NICK_REQ);
	cur += 4;
	len = 4;

	// Prepare nick request.
	memcpy(cur, req->nick, sizeof(req->nick));
	len += sizeof(req->nick);

	// Write packet.
	if (gls_writen(fd, buf, len) < len) {
		return g_flub_toss("Unable to write nick request: '%s'",
			g_serr(errno));
	}
	return NULL;
}

struct flub* gls_nick_validate(char* nick, int empty) {
	int i;

	// Validate nick characters.
	for (i = 0; i < GLS_NAME_LENGTH; i++) {
		if (nick[i] == '\0') {
			break;
		}
		if (!isalnum(nick[i])) {
			return g_flub_toss("Invalid character in nick at "
				"index '%i'", i);
		}
	}
	if (i == GLS_NAME_LENGTH) {
		return g_flub_toss("Nick exceeded '%i' characters",
			GLS_NAME_LENGTH - 1);
	}
	if (!empty && !strlen(nick)) {
		return g_flub_toss("Empty nick");
	}
	return NULL;
}

struct flub* gls_packet_read(struct gls_packet* packet, int fd, int validate) {
	struct flub* flub;

	// Read in packet header.
	memset(packet, 0, sizeof(struct gls_packet));
	flub = gls_header_read(&packet->header, fd);
	if (flub) {
		return flub;
	}

	// Read in actual packet.
	switch(packet->header.event) {
	case GLS_EVENT_PROTOVER:
		flub = gls_protover_read(&packet->data.protover, fd, validate);
		break;
	case GLS_EVENT_PROTOVERACK:
		flub = gls_protoverack_read(&packet->data.protoverack, fd,
			validate);
		break;
	case GLS_EVENT_NICK_REQ:
		flub = gls_nick_req_read(&packet->data.nick_req, fd, validate);
		break;
	case GLS_EVENT_NICK_SET:
		flub = gls_nick_set_read(&packet->data.nick_set, fd,
			validate);
		break;
	case GLS_EVENT_NICK_CHANGE:
		flub = gls_nick_change_read(&packet->data.nick_change, fd,
			validate);
		break;
	case GLS_EVENT_PLAYER_JOIN:
		flub = gls_player_join_read(&packet->data.player_join, fd,
			validate);
		break;
	case GLS_EVENT_PLAYER_PART:
		flub = gls_player_part_read(&packet->data.player_part, fd,
			validate);
		break;
	case GLS_EVENT_SHUTDOWN:
		flub = gls_shutdown_read(&packet->data.shutdown, fd, validate);
		break;
	case GLS_EVENT_SAY1:
		flub = gls_say1_read(&packet->data.say1, fd, validate);
		break;
	case GLS_EVENT_SAY2:
		flub = gls_say2_read(&packet->data.say2, fd, validate);
		break;
	default:
		flub = g_flub_toss("Unknown packet type: '%u'",
			packet->header.event);
		break;
	}
	if (flub) {
		return flub;
	}
	return NULL;
}

struct flub* gls_packet_write(struct gls_packet* packet, int fd) {
	struct flub* flub;

	// Write the packet.
	switch(packet->header.event) {
	case GLS_EVENT_PROTOVER:
		flub = gls_protover_write(&packet->data.protover, fd);
		break;
	case GLS_EVENT_PROTOVERACK:
		flub = gls_protoverack_write(&packet->data.protoverack, fd);
		break;
	case GLS_EVENT_NICK_REQ:
		flub = gls_nick_req_write(&packet->data.nick_req, fd);
		break;
	case GLS_EVENT_NICK_SET:
		flub = gls_nick_set_write(&packet->data.nick_set, fd);
		break;
	case GLS_EVENT_NICK_CHANGE:
		flub = gls_nick_change_write(&packet->data.nick_change, fd);
		break;
	case GLS_EVENT_PLAYER_JOIN:
		flub = gls_player_join_write(&packet->data.player_join, fd);
		break;
	case GLS_EVENT_PLAYER_PART:
		flub = gls_player_part_write(&packet->data.player_part, fd);
		break;
	case GLS_EVENT_SHUTDOWN:
		flub = gls_shutdown_write(&packet->data.shutdown, fd);
		break;
	case GLS_EVENT_SAY1:
		flub = gls_say1_write(&packet->data.say1, fd);
		break;
	case GLS_EVENT_SAY2:
		flub = gls_say2_write(&packet->data.say2, fd);
		break;
	default:
		flub = g_flub_toss("Unknown packet type: '%u'",
			packet->header.event);
		break;
	}
	if (flub) {
		return flub;
	}
	return NULL;
}

struct flub* gls_player_join_read(struct gls_player_join* join, int fd,
	int validate) {
	struct flub* flub;
	ssize_t size = sizeof(join->nick);

	// Read in data.
	if (gls_readn(fd, join->nick, size) < size) {
		return g_flub_toss("Unable to read player join nick: '%s'",
			g_serr(errno));
	}

	// Validate data.
	if (!validate) {
		return NULL;
	}
	if ((flub = gls_nick_validate(join->nick, 0))) {
		return flub_append(flub, "reading player join");
	}
	return NULL;
}

struct flub* gls_player_join_write(struct gls_player_join* join, int fd) {
	char* buf;
	char* cur;
	int32_t len;

	// Get buffer.
	buf = pthread_getspecific(gls_key);
	if (!buf) {
		return g_flub_toss("Unable to get buffer");
	}

	// Prepare header.
	cur = buf;
	gls_header_marshal(cur, GLS_EVENT_PLAYER_JOIN);
	cur += len = sizeof(len);

	// Prepare player join.
	memcpy(cur, join->nick, sizeof(join->nick));
	len += sizeof(join->nick);

	// Write data.
	if (gls_writen(fd, buf, len) < len) {
		return g_flub_toss("Unable to write player join: '%s'",
			g_serr(errno));
	}
	return NULL;
}

struct flub* gls_player_part_read(struct gls_player_part* part, int fd,
	int validate) {
	struct flub* flub;
	ssize_t size = sizeof(part->nick);

	// Read in data.
	if (gls_readn(fd, part->nick, size) < size) {
		return g_flub_toss("Unable to read player part nick: '%s'",
			g_serr(errno));
	}

	// Validate data.
	if (!validate) {
		return NULL;
	}
	if ((flub = gls_nick_validate(part->nick, 0))) {
		return flub_append(flub, "reading player part");
	}
	return NULL;
}

struct flub* gls_player_part_write(struct gls_player_part* part, int fd) {
	char* buf;
	char* cur;
	int32_t len;

	// Get buffer.
	buf = pthread_getspecific(gls_key);
	if (!buf) {
		return g_flub_toss("Unable to get buffer");
	}

	// Prepare header.
	cur = buf;
	gls_header_marshal(cur, GLS_EVENT_PLAYER_PART);
	cur += len = sizeof(len);

	// Prepare player part.
	memcpy(cur, part->nick, sizeof(part->nick));
	len += sizeof(part->nick);

	// Write data.
	if (gls_writen(fd, buf, len) < len) {
		return g_flub_toss("Unable to write player part: '%s'",
			g_serr(errno));
	}
	return NULL;
}

struct flub* gls_protover_read(struct gls_protover* pver, int fd,
	int validate) {
	struct iovec iovs[3];
	int i;
	ssize_t len;

	// Read protover.
	memset(pver, 0, sizeof(struct gls_protover));
	iovs[0].iov_base = &pver->magic;
	iovs[0].iov_len = GLS_PROTOVER_MAGIC_LENGTH;
	iovs[1].iov_base = &pver->version;
	iovs[1].iov_len = GLS_PROTOVER_VERSION_LENGTH;
	iovs[2].iov_base = &pver->software;
	iovs[2].iov_len = GLS_PROTOVER_SOFTWARE_LENGTH;
	len = GLS_PROTOVER_MAGIC_LENGTH + GLS_PROTOVER_VERSION_LENGTH +
		GLS_PROTOVER_SOFTWARE_LENGTH;
	if (gls_readvn(fd, iovs, 3) < len) {
		return g_flub_toss("Unable to read protover: '%s'",
			g_serr(errno));
	}
	pver->magic[GLS_PROTOVER_MAGIC_LENGTH - 1] = '\0';
	pver->version[GLS_PROTOVER_VERSION_LENGTH - 1] = '\0';
	pver->software[GLS_PROTOVER_SOFTWARE_LENGTH - 1] = '\0';

	// Validate protover.
	if (!validate) {
		return NULL;
	}
	if (strncmp(pver->magic, "GLS", GLS_PROTOVER_MAGIC_LENGTH)) {
		return g_flub_toss("Invalid magic in protover");
	}
	for (i = 0; i < strlen(pver->version); i++) {
		if ((!isdigit(pver->version[i])) && (pver->version[i] != '.')) {
			return g_flub_toss("Invalid version in protover");
		}
	}
	for (i = 0; i < strlen(pver->software); i++) {
		if ((!isalnum(pver->software[i])) &&
			(pver->software[i] != '_')) {
			return g_flub_toss("Invalid software in protover");
		}
	}
	return NULL;
}

struct flub* gls_protover_write(struct gls_protover* pver, int fd) {
	char* buf;
	char* cur;
	ssize_t len;

	// Get buffer.
	buf = pthread_getspecific(gls_key);
	if (!buf) {
		return g_flub_toss("Unable to get buffer");
	}

	// Prepare data.
	cur = buf;
	gls_header_marshal(cur, GLS_EVENT_PROTOVER);
	cur += 4;
	len = 4;
	memcpy(cur, pver->magic, GLS_PROTOVER_MAGIC_LENGTH);
	len += GLS_PROTOVER_MAGIC_LENGTH;
	cur += GLS_PROTOVER_MAGIC_LENGTH;
	memcpy(cur, pver->version, GLS_PROTOVER_VERSION_LENGTH);
	len += GLS_PROTOVER_VERSION_LENGTH;
	cur += GLS_PROTOVER_VERSION_LENGTH;
	memcpy(cur, pver->software, GLS_PROTOVER_SOFTWARE_LENGTH);
	len += GLS_PROTOVER_SOFTWARE_LENGTH;

	// Write packet.
	if (gls_writen(fd, buf, len) < len) {
		return g_flub_toss("Unable to write full protover");
	}
	return NULL;
}

struct flub* gls_protoverack_read(struct gls_protoverack* pack, int fd,
	int validate) {
	struct flub* flub;
	int i;
	struct iovec iovs[2];
	ssize_t len;

	// Read in first part.
	memset(pack, 0, sizeof(struct gls_protoverack));
	iovs[0].iov_base = &pack->ack;
	len = iovs[0].iov_len = sizeof(uint16_t);
	iovs[1].iov_base = &pack->reason;
	len += iovs[1].iov_len = GLS_PROTOVER_REASON_LENGTH;
	if (gls_readvn(fd, iovs, 2) < len) {
		return g_flub_toss("Unable to read ack header: '%s'",
			g_serr(errno));
	}
	pack->ack = ntohs(pack->ack);
	pack->reason[GLS_PROTOVER_REASON_LENGTH - 1] = '\0';

	// Read in protover.
	flub = gls_protover_read(&pack->pver, fd, validate);
	if (flub) {
		return flub_append(flub, "unable to read protoack");
	}

	// Validate reason.
	if (!validate) {
		return NULL;
	}
	for (i = 0; i < GLS_PROTOVER_REASON_LENGTH; i++) {
		if (pack->reason[i] && (!isprint(pack->reason[i]))) {
			return g_flub_toss("Invalid reason in protover ack");
		}
	}
	return NULL;
}

struct flub* gls_protoverack_write(struct gls_protoverack* pack, int fd) {
	char* buf;
	char* cur;
	ssize_t len;
	uint16_t tmp;

	// Get buffer.
	buf = pthread_getspecific(gls_key);
	if (!buf) {
		return g_flub_toss("Unable to get buffer");
	}

	// Prepare ack and reason.
	cur = buf;
	gls_header_marshal(cur, GLS_EVENT_PROTOVERACK);
	cur += 4;
	len = 4;
	tmp = htons(pack->ack);
	memcpy(cur, &tmp, sizeof(uint16_t));
	len += sizeof(uint16_t);
	cur += sizeof(uint16_t);
	memcpy(cur, pack->reason, GLS_PROTOVER_REASON_LENGTH);
	len += GLS_PROTOVER_REASON_LENGTH;
	cur += GLS_PROTOVER_REASON_LENGTH;
	memcpy(cur, pack->pver.magic, GLS_PROTOVER_MAGIC_LENGTH);
	len += GLS_PROTOVER_MAGIC_LENGTH;
	cur += GLS_PROTOVER_MAGIC_LENGTH;
	memcpy(cur, pack->pver.version, GLS_PROTOVER_VERSION_LENGTH);
	len += GLS_PROTOVER_VERSION_LENGTH;
	cur += GLS_PROTOVER_VERSION_LENGTH;
	memcpy(cur, pack->pver.software, GLS_PROTOVER_SOFTWARE_LENGTH);
	len += GLS_PROTOVER_SOFTWARE_LENGTH;

	// Write packet.
	if (gls_writen(fd, buf, len) < len) {
		return g_flub_toss("Unable to write protoverack pt1: '%s'",
			g_serr(errno));
	}
	return NULL;
}

ssize_t gls_readn(int fd, void* buffer, size_t count) {
	return gls_rdwrn(fd, buffer, count, read);
}

ssize_t gls_readvn(int fd, struct iovec* iov, int iovcnt) {
	return gls_rdwrvn(fd, iov, iovcnt, readv);
}

struct flub* gls_say_message_validate(char* message) {
	int i;

	// Validate message.
	for (i = 0; i < GLS_SAY_MESSAGE_LENGTH; i++) {
		if (message[i] == '\0') {
			break;
		}
		if (!isprint(message[i])) {
			return g_flub_toss("Invalid Say message character at "
				"index '%i'", i);
		}
	}
	if (i == GLS_SAY_MESSAGE_LENGTH) {
		return g_flub_toss("Unterminated message");
	} else if (!i) {
		return g_flub_toss("Empty message");
	}
	return NULL;
}

struct flub* gls_say1_read(struct gls_say1* say, int fd, int validate) {
	struct flub* flub;

	// Read packet.
	memset(say, 0, sizeof(struct gls_say1));
	if (gls_readn(fd, say->message, GLS_SAY_MESSAGE_LENGTH) <
		GLS_SAY_MESSAGE_LENGTH) {
		return g_flub_toss("Unable to read Say1 packet: '%s'",
			g_serr(errno));
	}

	// Validate data.
	if (!validate) {
		return NULL;
	}
	if ((flub = gls_say_message_validate(say->message))) {
		return flub_append(flub, "reading say1 packet");
	}
	return NULL;
}

struct flub* gls_say1_write(struct gls_say1* say, int fd) {
	char* buf;
	char* cur;
	size_t len;

	// Get buffer.
	if (!(buf = pthread_getspecific(gls_key))) {
		return g_flub_toss("Unable to get buffer");
	}

	// Marshal header.
	cur = buf;
	gls_header_marshal(cur, GLS_EVENT_SAY1);
	cur += len = 4;

	// Prepare buffer.
	strlcpy(cur, say->message, GLS_SAY_MESSAGE_LENGTH);
	cur += GLS_SAY_MESSAGE_LENGTH;
	len += GLS_SAY_MESSAGE_LENGTH;

	// Write buffer.
	if (gls_writen(fd, buf, len) < len) {
		return g_flub_toss("Unable to write Say1 packet");
	}
	return NULL;
}

struct flub* gls_say2_read(struct gls_say2* say, int fd, int validate) {
	struct flub* flub;
	struct iovec iovs[3];
	size_t len;

	len = 0;
	iovs[0].iov_base = &say->nick;
	len += iovs[0].iov_len = GLS_NAME_LENGTH;
	iovs[1].iov_base = &say->tval;
	len += iovs[1].iov_len = sizeof(uint64_t);
	iovs[2].iov_base = &say->message;
	len += iovs[2].iov_len = GLS_SAY_MESSAGE_LENGTH;

	// Read in data.
	memset(say, 0, sizeof(struct gls_say2));
	if (gls_readvn(fd, iovs, 3) < len) {
		return g_flub_toss("Unable to read Say2 packet: '%s'",
			g_serr(errno));
	}
	say->tval = be64toh(say->tval);

	// Validate data.
	if (!validate) {
		return NULL;
	}
	if ((flub = gls_nick_validate(say->nick, 0))) {
		return flub_append(flub, "reading Say2 packet");
	}
	if ((flub = gls_say_message_validate(say->message))) {
		return flub_append(flub, "reading Say2 packet");
	}
	return NULL;
}

struct flub* gls_say2_write(struct gls_say2* say, int fd) {
	char* buf;
	char* cur;
	size_t len;
	uint64_t time;

	// Get buffer.
	buf = pthread_getspecific(gls_key);
	if (!buf) {
		return g_flub_toss("Unable to get buffer");
	}
	cur = buf;

	// Marshal header.
	gls_header_marshal(cur, GLS_EVENT_SAY2);
	cur += len = 4;

	// Prepare buffer.
	strlcpy(cur, say->nick, GLS_NAME_LENGTH);
	cur += GLS_NAME_LENGTH;
	len += GLS_NAME_LENGTH;
	time = htobe64(say->tval);
	memcpy(cur, &time, sizeof(uint64_t));
	cur += sizeof(uint64_t);
	len += sizeof(uint64_t);
	strlcpy(cur, say->message, GLS_SAY_MESSAGE_LENGTH);
	len += GLS_SAY_MESSAGE_LENGTH;

	// Write buffer.
	if (gls_writen(fd, buf, len) < len) {
		return g_flub_toss("Unable to write Say2 packet");
	}
	return NULL;
}

struct flub* gls_shutdown_read(struct gls_shutdown* shutdown, int fd,
	int validate) {
	int i;
	size_t len;

	// Read in data.
	memset(shutdown, 0, sizeof(struct gls_shutdown));
	if (gls_readn(fd, &shutdown->reason, GLS_SHUTDOWN_REASON_LENGTH) <
		GLS_SHUTDOWN_REASON_LENGTH) {
		return g_flub_toss("Unable to read Shutdown packet: '%s'",
			g_serr(errno));
	}

	// Validate data.
	if (!validate) {
		return NULL;
	}
	shutdown->reason[GLS_SHUTDOWN_REASON_LENGTH - 1] = '\0';
	if (!(len = strlen(shutdown->reason))) {
		return NULL;
	}
	for (i = 0; i < len; i++) {
		if (!isprint(shutdown->reason[i])) {
			return g_flub_toss("Invalid shutdown reason char at "
				"'%i'", i);
		}
	}
	return NULL;
}

struct flub* gls_shutdown_write(struct gls_shutdown* shutdown, int fd) {
	char* buf;
	char* cur;
	int32_t len;

	// Get buffer.
	buf = pthread_getspecific(gls_key);
	if (!buf) {
		return g_flub_toss("Unable to get buffer");
	}

	// Prepare header.
	cur = buf;
	gls_header_marshal(cur, GLS_EVENT_SHUTDOWN);
	cur += len = sizeof(len);

	// Prepare player part.
	memcpy(cur, shutdown->reason, GLS_SHUTDOWN_REASON_LENGTH);
	len += GLS_SHUTDOWN_REASON_LENGTH;

	// Write data.
	if (gls_writen(fd, buf, len) < len) {
		return g_flub_toss("Unable to write shutdown: '%s'",
			g_serr(errno));
	}
	return NULL;
}

ssize_t gls_writen(int fd, void* buffer, size_t count) {
	return gls_rdwrn(fd, buffer, count,
		(ssize_t(*)(int, void*, size_t))write);
}

ssize_t gls_writevn(int fd, struct iovec* iov, int iovcnt) {
	return gls_rdwrvn(fd, iov, iovcnt, writev);
}
