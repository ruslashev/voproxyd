#pragma once

#include "deps/onvif/wsddapi.h"

soap_wsdd_mode wsdd_event_Probe(struct soap *soap, const char *message_id, const char *reply_to,
        const char *types, const char *scopes, const char *match_by,
        struct wsdd__ProbeMatchesType *matches);

void wsdd_event_ProbeMatches(struct soap *soap, unsigned int instance_id, const char *sequence_id,
        unsigned int message_number, const char *message_id, const char *relates_to,
        struct wsdd__ProbeMatchesType *matches);

soap_wsdd_mode wsdd_event_Resolve(struct soap *soap, const char *message_id, const char *reply_to,
        const char *endpoint_ref, struct wsdd__ResolveMatchType *match);

void wsdd_event_ResolveMatches(struct soap *soap, unsigned int instance_id, const char *sequence_id,
        unsigned int message_number, const char *message_id, const char *relates_to,
        struct wsdd__ResolveMatchType *match);

void wsdd_event_Hello(struct soap *soap, unsigned int instance_id, const char *sequence_id,
        unsigned int message_number, const char *message_id, const char *relates_to,
        const char *endpoint_ref, const char *types, const char *scopes, const char *match_by,
        const char *XAddrs, unsigned int metadata_version);

void wsdd_event_Bye(struct soap *soap, unsigned int instance_id, const char *sequence_id,
        unsigned int message_number, const char *message_id, const char *relates_to,
        const char *endpoint_ref, const char *types, const char *scopes, const char *match_by,
        const char *XAddrs, unsigned int *metadata_version);

