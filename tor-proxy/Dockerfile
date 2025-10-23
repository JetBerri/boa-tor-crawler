FROM alpine:latest

LABEL maintainer="Dioni Anton <jetberri@quersystem.net>"
LABEL description="Stable Tor SOCKS5 proxy for crawler"

RUN apk add --no-cache tor netcat-openbsd

COPY torrc /etc/tor/torrc

RUN mkdir -p /var/lib/tor && chown -R tor:tor /var/lib/tor

EXPOSE 9050 9051

USER tor

ENTRYPOINT ["tor", "-f", "/etc/tor/torrc"]
