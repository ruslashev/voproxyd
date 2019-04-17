#include "wsdd_callbacks.h"
#include "log.h"
#include "address_manager.h"

#define log_match(X) do { \
    if ((X)->wsa5__EndpointReference.Address) \
        log("endpoint:\t%s", (X)->wsa5__EndpointReference.Address); \
    if ((X)->Types) \
        log("types:\t\t%s", (X)->Types); \
    if ((X)->Scopes) { \
        if ((X)->Scopes->__item ) \
            log("scopes:\t\t%s", (X)->Scopes->__item); \
        if ((X)->Scopes->MatchBy) \
            log("matchBy:\t%s", (X)->Scopes->MatchBy); \
    } \
    if ((X)->XAddrs) \
        log("xaddrs:\t\t%s", (X)->XAddrs); \
    log("MetadataVersion:\t\t%u", (X)->MetadataVersion); \
} while (0);

soap_wsdd_mode wsdd_event_Probe(struct soap *soap, const char *message_id, const char *reply_to,
        const char *types, const char *scopes, const char *match_by,
        struct wsdd__ProbeMatchesType *matches)
{
    return SOAP_WSDD_ADHOC;
}

void wsdd_event_ProbeMatches(struct soap *soap, unsigned int instance_id, const char *sequence_id,
        unsigned int message_number, const char *message_id, const char *relates_to,
        struct wsdd__ProbeMatchesType *matches)
{
    log("wsdd probe_matches");
    log("iid\t%u", instance_id);
    log("sid\t%s", sequence_id);
    log("mn\t%u", message_number);
    log("mid\t%s", message_id);
    log("relates\t%s", relates_to);
    log(" ");

    for (int i = 0; i < matches->__sizeProbeMatch; ++i) {
        log("match %d/%d", i + 1, matches->__sizeProbeMatch);
        log_match(&matches->ProbeMatch[i]);
        log(" ");

        address_mngr_add_address();
    }
}

soap_wsdd_mode wsdd_event_Resolve(struct soap *soap, const char *message_id, const char *reply_to,
        const char *endpoint_ref, struct wsdd__ResolveMatchType *match)
{
    log("wsdd resolve");

    return SOAP_WSDD_ADHOC;
}

void wsdd_event_ResolveMatches(struct soap *soap, unsigned int instance_id, const char *sequence_id,
        unsigned int message_number, const char *message_id, const char *relates_to,
        struct wsdd__ResolveMatchType *match)
{
    log("wsdd resolve_matches iid %u sid %s mn %u mid %s relates %s", instance_id, sequence_id,
            message_number, message_id, relates_to);

    log_match(match);
}

void wsdd_event_Hello(struct soap *soap, unsigned int instance_id, const char *sequence_id,
        unsigned int message_number, const char *message_id, const char *relates_to,
        const char *endpoint_ref, const char *types, const char *scopes, const char *match_by,
        const char *XAddrs, unsigned int metadata_version)
{
    log("wsdd hello");
}

void wsdd_event_Bye(struct soap *soap, unsigned int instance_id, const char *sequence_id,
        unsigned int message_number, const char *message_id, const char *relates_to,
        const char *endpoint_ref, const char *types, const char *scopes, const char *match_by,
        const char *XAddrs, unsigned int *metadata_version)
{
    log("wsdd bye");
}

