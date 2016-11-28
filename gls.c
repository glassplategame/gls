#include "gls.h"

static char gls_buffer[65536];

void gls_header_marshal(char* buffer, uint32_t event) {
	uint32_t tmp;

	// Write header data.
	tmp = htonl(event);
	memcpy(buffer, &tmp, sizeof(uint32_t));
}

int gls_header_read(struct gls_header* header, int fd) {
	// Read header.
	if (read(fd, &header->event, sizeof(uint32_t)) < sizeof(uint32_t)) {
		// Failed/partial read.
		log_error(&g_log, g_serror("Unable to read header"));
		return -1;
	}
	header->event = ntohl(header->event);
	return 0;
}

int gls_nick_reply_read(struct gls_nick_reply* reply, int fd) {
	struct iovec iovs[2];

	// Read nick reply.
	memset(reply, 0, sizeof(struct gls_nick_reply));
	iovs[0].iov_base = &reply->nick;
	iovs[0].iov_len = PLAYER_NAME_LENGTH;
	iovs[1].iov_base = &reply->accepted;
	iovs[1].iov_len = sizeof(uint16_t);
	if (readv(fd, iovs, 2) < PLAYER_NAME_LENGTH + sizeof(uint16_t)) {
		log_error(&g_log, g_serror("Unable to read nick reply"));
		return -1;
	}
	reply->accepted = ntohs(reply->accepted);
	return 0;
}

int gls_nick_reply_write(struct gls_nick_reply* reply, int fd) {
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
	if (write(fd, gls_buffer, len) < len) {
		log_error(&g_log, g_serror("Unable to write nick reply"));
		return -1;
	}
	return 0;
}

int gls_nick_req_read(struct gls_nick_req* req, int fd) {
	int i;
	size_t size = sizeof(req->nick);

	// Read nick request.
	memset(req, 0, sizeof(struct gls_nick_req));
	if (read(fd, req->nick, size) < size) {
		log_error(&g_log, g_serror("Unable to read nick request"));
		return -1;
	}

	// Validate nick.
	for (i = 0; i < PLAYER_NAME_LENGTH - 1; i++) {
		if ((!isalnum(req->nick[i])) && (req->nick[i] != '\0')) {
			log_error(&g_log, "Invalid character in nick");
			return -1;
		}
	}
	req->nick[i] = '\0';
	return 0;
}

int gls_nick_req_write(struct gls_nick_req* req, int fd) {
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
	// TODO: Need writen.
	if (write(fd, gls_buffer, len) < len) {
		log_error(&g_log, g_serror("Unable to write nick request"));
		return -1;
	}
	return 0;
}

int gls_protover_read(struct gls_protover* pver, int fd) {
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
	if (readv(fd, iovs, 3) < len) {
		log_error(&g_log, "Unable to read protover");
		return -1;
	}
	pver->magic[GLS_PROTOVER_MAGIC_LENGTH - 1] = '\0';
	pver->version[GLS_PROTOVER_VERSION_LENGTH - 1] = '\0';
	pver->software[GLS_PROTOVER_SOFTWARE_LENGTH - 1] = '\0';

	// Verify protover.
	if (strncmp(pver->magic, "GLS", GLS_PROTOVER_MAGIC_LENGTH)) {
		log_error(&g_log, "Invalid magic in protover");
		return -1;
	}
	for (i = 0; i < strlen(pver->version); i++) {
		if ((!isdigit(pver->version[i])) && (pver->version[i] != '.')) {
			log_error(&g_log, "Invalid version in protover");
			return -1;
		}
	}
	for (i = 0; i < strlen(pver->software); i++) {
		if ((!isalnum(pver->software[i])) &&
			(pver->software[i] != '_')) {
			log_error(&g_log, "Invalid software in protover");
			return -1;
		}
	}
	return 0;
}

int gls_protover_write(struct gls_protover* pver, int fd) {
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
	if (write(fd, gls_buffer, len) < len) {
		log_error(&g_log, "Unable to write full protover");
		return -1;
	}
	return 0;
}

int gls_protoverack_read(struct gls_protoverack* pack, int fd) {
	int i;
	struct iovec iovs[2];
	size_t len;

	// Read in first part.
	memset(pack, 0, sizeof(struct gls_protoverack));
	iovs[0].iov_base = &pack->ack;
	len = iovs[0].iov_len = sizeof(uint16_t);
	iovs[1].iov_base = &pack->reason;
	len += iovs[1].iov_len = GLS_PROTOVER_REASON_LENGTH;
	if (readv(fd, iovs, 2) < len) {
		log_error(&g_log, "Unable to read ack header");
		return -1;
	}
	pack->ack = ntohs(pack->ack);
	pack->reason[GLS_PROTOVER_REASON_LENGTH - 1] = '\0';

	// Read in protover.
	if (gls_protover_read(&pack->pver, fd) == -1) {
		log_error(&g_log, "Unable to read protoack");
		return -1;
	}

	// Validate reason.
	for (i = 0; i < GLS_PROTOVER_REASON_LENGTH; i++) {
		if ((!isalnum(pack->reason[i])) && (pack->reason[i] != ' ') &&
			pack->reason[i]) {
			log_error(&g_log, "Invalid reason in protover ack");
			return -1;
		}
	}
	return 0;
}

int gls_protoverack_write(struct gls_protoverack* pack, int fd) {
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
	if (write(fd, gls_buffer, len) < len) {
		log_error(&g_log, "Unable to write protoverack pt1");
		return -1;
	}
	if (gls_protover_write(&pack->pver, fd) == -1) {
		log_error(&g_log, "Unable to write protoverack pt2");
		return -1;
	}
	return 0;
}
