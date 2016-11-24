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
	if (read(fd, &header->event, 4) < 4) {
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
