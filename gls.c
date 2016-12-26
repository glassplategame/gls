#include "gls.h"

static char gls_buffer[65536];

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
		return g_flub_toss(g_serror("Unable to read header"));
	}
	header->event = ntohl(header->event);
	return NULL;
}

struct flub* gls_nick_reply_read(struct gls_nick_reply* reply, int fd) {
	struct iovec iovs[2];

	// Read nick reply.
	memset(reply, 0, sizeof(struct gls_nick_reply));
	iovs[0].iov_base = &reply->nick;
	iovs[0].iov_len = PLAYER_NAME_LENGTH;
	iovs[1].iov_base = &reply->accepted;
	iovs[1].iov_len = sizeof(uint16_t);
	if (gls_readvn(fd, iovs, 2) < PLAYER_NAME_LENGTH + sizeof(uint16_t)) {
		return g_flub_toss(g_serror("Unable to read nick reply"));
	}
	reply->accepted = ntohs(reply->accepted);
	return NULL;
}

struct flub* gls_nick_reply_write(struct gls_nick_reply* reply, int fd) {
	char* cur = gls_buffer;
	uint32_t len;
	uint16_t tmp;

	// Prepare header.
	gls_header_marshal(cur, GLS_EVENT_NICK_REPLY);
	cur += 4;
	len = 4;

	// Prepare nick reply.
	memcpy(cur, reply->nick, PLAYER_NAME_LENGTH);
	cur += PLAYER_NAME_LENGTH;
	len += PLAYER_NAME_LENGTH;
	tmp = htons(reply->accepted);
	memcpy(cur, &tmp, sizeof(uint16_t));
	cur += sizeof(uint16_t);
	len += sizeof(uint16_t);

	// Write packet.
	if (gls_writen(fd, gls_buffer, len) < len) {
		return g_flub_toss(g_serror("Unable to write nick reply"));
	}
	return NULL;
}

struct flub* gls_nick_req_read(struct gls_nick_req* req, int fd) {
	int i;
	size_t size = sizeof(req->nick);

	// Read nick request.
	memset(req, 0, sizeof(struct gls_nick_req));
	if (gls_readn(fd, req->nick, size) < size) {
		return g_flub_toss(g_serror("Unable to read nick request"));
	}

	// Validate nick.
	for (i = 0; i < PLAYER_NAME_LENGTH - 1; i++) {
		if ((!isalnum(req->nick[i])) && (req->nick[i] != '\0')) {
			return g_flub_toss("Invalid character in nick");
		}
	}
	req->nick[i] = '\0';
	return NULL;
}

struct flub* gls_nick_req_write(struct gls_nick_req* req, int fd) {
	char* cur = gls_buffer;
	uint32_t len;

	// Prepare header.
	gls_header_marshal(cur, GLS_EVENT_NICK_REQ);
	cur += 4;
	len = 4;

	// Prepare nick request.
	memcpy(cur, req->nick, sizeof(req->nick));
	len += sizeof(req->nick);

	// Write packet.
	if (gls_writen(fd, gls_buffer, len) < len) {
		return g_flub_toss(g_serror("Unable to write nick request"));
	}
	return NULL;
}

struct flub* gls_protover_read(struct gls_protover* pver, int fd) {
	struct iovec iovs[3];
	int i;
	size_t len;

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
		return g_flub_toss(g_serror("Unable to read protover"));
	}
	pver->magic[GLS_PROTOVER_MAGIC_LENGTH - 1] = '\0';
	pver->version[GLS_PROTOVER_VERSION_LENGTH - 1] = '\0';
	pver->software[GLS_PROTOVER_SOFTWARE_LENGTH - 1] = '\0';

	// Verify protover.
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
	char* cur = gls_buffer;
	size_t len;

	// Prepare data.
	len = 0;
	memcpy(cur, pver->magic, GLS_PROTOVER_MAGIC_LENGTH);
	len += GLS_PROTOVER_MAGIC_LENGTH;
	cur += GLS_PROTOVER_MAGIC_LENGTH;
	memcpy(cur, pver->version, GLS_PROTOVER_VERSION_LENGTH);
	len += GLS_PROTOVER_VERSION_LENGTH;
	cur += GLS_PROTOVER_VERSION_LENGTH;
	memcpy(cur, pver->software, GLS_PROTOVER_SOFTWARE_LENGTH);
	len += GLS_PROTOVER_SOFTWARE_LENGTH;

	// Write packet.
	if (gls_writen(fd, gls_buffer, len) < len) {
		return g_flub_toss("Unable to write full protover");
	}
	return NULL;
}

struct flub* gls_protoverack_read(struct gls_protoverack* pack, int fd) {
	struct flub* flub;
	int i;
	struct iovec iovs[2];
	size_t len;

	// Read in first part.
	memset(pack, 0, sizeof(struct gls_protoverack));
	iovs[0].iov_base = &pack->ack;
	len = iovs[0].iov_len = sizeof(uint16_t);
	iovs[1].iov_base = &pack->reason;
	len += iovs[1].iov_len = GLS_PROTOVER_REASON_LENGTH;
	if (gls_readvn(fd, iovs, 2) < len) {
		return g_flub_toss(g_serror("Unable to read ack header"));
	}
	pack->ack = ntohs(pack->ack);
	pack->reason[GLS_PROTOVER_REASON_LENGTH - 1] = '\0';

	// Read in protover.
	flub = gls_protover_read(&pack->pver, fd);
	if (flub) {
		return flub_append(flub, "unable to read protoack");
	}

	// Validate reason.
	for (i = 0; i < GLS_PROTOVER_REASON_LENGTH; i++) {
		if (pack->reason[i] && (!isprint(pack->reason[i]))) {
			return g_flub_toss("Invalid reason in protover ack");
		}
	}
	return NULL;
}

struct flub* gls_protoverack_write(struct gls_protoverack* pack, int fd) {
	struct flub* flub;
	char* cur = gls_buffer;
	size_t len;
	uint16_t tmp;

	// Prepare ack and reason.
	tmp = htons(pack->ack);
	memcpy(cur, &tmp, sizeof(uint16_t));
	cur += len = sizeof(uint16_t);
	memcpy(cur, pack->reason, GLS_PROTOVER_REASON_LENGTH);
	len += GLS_PROTOVER_REASON_LENGTH;
	cur += GLS_PROTOVER_REASON_LENGTH;

	// Write packet.
	if (gls_writen(fd, gls_buffer, len) < len) {
		return g_flub_toss(g_serror("Unable to write protoverack pt1"));
	}
	flub = gls_protover_write(&pack->pver, fd);
	if (flub) {
		return flub_append(flub, "unable to write protoverack pt2");
	}
	return NULL;
}

ssize_t gls_readn(int fd, void* buffer, size_t count) {
	return gls_rdwrn(fd, buffer, count, read);
}

ssize_t gls_readvn(int fd, struct iovec* iov, int iovcnt) {
	return gls_rdwrvn(fd, iov, iovcnt, readv);
}

ssize_t gls_writen(int fd, void* buffer, size_t count) {
	return gls_rdwrn(fd, buffer, count,
		(ssize_t(*)(int, void*, size_t))write);
}

ssize_t gls_writevn(int fd, struct iovec* iov, int iovcnt) {
	return gls_rdwrvn(fd, iov, iovcnt, writev);
}
