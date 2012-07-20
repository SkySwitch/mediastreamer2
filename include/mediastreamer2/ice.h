/*
mediastreamer2 library - modular sound and video processing and streaming
Copyright (C) 2012  Belledonne Communications

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef ice_h
#define ice_h

#include "mscommon.h"
#include "msticker.h"
#include "ortp/stun_udp.h"
#include "ortp/stun.h"
#include "ortp/ortp.h"


/**
 * @file ice.h
 * @brief mediastreamer2 ice.h include file
 *
 * This file provides the API to handle the ICE protocol defined in the RFC 5245.
 */


/**
 * ICE agent role.
 *
 * See the terminology in paragraph 3 of the RFC 5245 for more details.
 */
typedef enum {
	IR_Controlling,
	IR_Controlled
} IceRole;

/**
 * ICE candidate type.
 *
 * See the terminology in paragraph 3 of the RFC 5245 for more details.
 */
typedef enum {
	ICT_HostCandidate,
	ICT_ServerReflexiveCandidate,
	ICT_PeerReflexiveCandidate,
	ICT_RelayedCandidate
} IceCandidateType;

/**
 * ICE candidate pair state.
 *
 * See paragraph 5.7.4 ("Computing states") of RFC 5245 for more details.
 */
typedef enum {
	ICP_Waiting,
	ICP_InProgress,
	ICP_Succeeded,
	ICP_Failed,
	ICP_Frozen
} IceCandidatePairState;

/**
 * ICE check list state.
 *
 * See paragraph 5.7.4 ("Computing states") of RFC 5245 for more details.
 */
typedef enum {
	ICL_Running,
	ICL_Completed,
	ICL_Failed
} IceCheckListState;

/**
 * ICE session state.
 */
typedef enum {
	IS_Stopped,
	IS_Running,
	IS_Completed,
	IS_Failed
} IceSessionState;

/**
 * Structure representing an ICE session.
 */
typedef struct _IceSession {
	MSList *streams;	/**< List of IceChecklist structures. Each element of the list represents a media stream */
	MSTicker *ticker;	/**< Ticker used to handle retransmissions of connectivity checks */
	char *local_ufrag;	/**< Local username fragment for the session (assigned during the session creation) */
	char *local_pwd;	/**< Local password for the session (assigned during the session creation) */
	char *remote_ufrag;	/**< Remote username fragment for the session (provided via SDP by the peer) */
	char *remote_pwd;	/**< Remote password for the session (provided via SDP by the peer) */
	IceRole role;	/**< Role played by the agent for this session */
	IceSessionState state;	/**< State of the session */
	uint64_t tie_breaker;	/**< Random number used to resolve role conflicts (see paragraph 5.2 of the RFC 5245) */
	uint32_t ta;	/**< Duration of timer for sending connectivity checks in ms */
	uint8_t max_connectivity_checks;	/**< Configuration parameter to limit the number of connectivity checks performed by the agent (default is 100) */
	uint8_t keepalive_timeout;	/**< Configuration parameter to define the timeout between each keepalive packets (default is 15s) */
} IceSession;

/**
 * Structure representing an ICE transport address.
 */
typedef struct _IceTransportAddress {
	char ip[64];
	int port;
	// TODO: Handling of IP version (4 or 6) and transport type: TCP, UDP...
} IceTransportAddress;

/**
 * Structure representing an ICE candidate.
 */
typedef struct _IceCandidate {
	char foundation[32];	/**< Foundation of the candidate (see paragraph 3 of the RFC 5245 for more details */
	IceTransportAddress taddr;	/**< Transport address of the candidate */
	IceCandidateType type;	/**< Type of the candidate */
	uint32_t priority;	/**< Priority of the candidate */
	uint16_t componentID;	/**< component ID between 1 and 256: usually 1 for RTP component and 2 for RTCP component */
	struct _IceCandidate *base;	/**< Pointer to the candidate that is the base of the current one */
	bool_t is_default;	/**< Boolean value telling whether this candidate is a default candidate or not */
} IceCandidate;

/**
 * Structure representing an ICE candidate pair.
 */
typedef struct _IceCandidatePair {
	IceCandidate *local;	/**< Pointer to the local candidate of the pair */
	IceCandidate *remote;	/**< Pointer to the remote candidate of the pair */
	IceCandidatePairState state;	/**< State of the candidate pair */
	uint64_t priority;	/**< Priority of the candidate pair */
	UInt96 transactionID;	/**< Transaction ID of the connectivity check sent for the candidate pair */
	uint64_t transmission_time;	/**< Time when the connectivity check for the candidate pair has been sent */
	uint32_t rto;	/**< Duration of the retransmit timer for the connectivity check sent for the candidate pair in ms */
	uint8_t retransmissions;	/**< Number of retransmissions for the connectivity check sent for the candidate pair */
	IceRole role;	/**< Role of the agent when the connectivity check has been sent for the candidate pair */
	bool_t is_default;	/**< Boolean value telling whether this candidate pair is a default candidate pair or not */
	bool_t is_nominated;	/**< Boolean value telling whether this candidate pair is nominated or not */
	bool_t wait_transaction_timeout;	/**< Boolean value telling to create a new binding request on retransmission timeout */
} IceCandidatePair;

/**
 * Structure representing the foundation of an ICE candidate pair.
 *
 * It is the concatenation of the foundation of a local candidate and the foundation of a remote candidate.
 */
typedef struct _IcePairFoundation {
	char local[32];	/**< Foundation of the local candidate */
	char remote[32];	/**< Foundation of the remote candidate */
} IcePairFoundation;

typedef struct _IceValidCandidatePair {
	IceCandidatePair *valid;	/**< Pointer to a valid candidate pair (it may be in the check list or not */
	IceCandidatePair *generated_from;	/**< Pointer to the candidate pair that generated the connectivity check producing the valid candidate pair */
} IceValidCandidatePair;

/**
 * Structure representing an ICE check list.
 *
 * Each media stream must be assigned a check list.
 * Check lists are added to an ICE session using the ice_session_add_check_list() function.
 */
typedef struct _IceCheckList {
	IceSession *session;	/**< Pointer to the ICE session */
	char *remote_ufrag;	/**< Remote username fragment for this check list (provided via SDP by the peer) */
	char *remote_pwd;	/**< Remote password for this check list (provided via SDP by the peer) */
	MSList *local_candidates;	/**< List of IceCandidate structures */
	MSList *remote_candidates;	/**< List of IceCandidate structures */
	MSList *pairs;	/**< List of IceCandidatePair structures */
	MSList *triggered_checks_queue;	/**< List of IceCandidatePair structures */
	MSList *check_list;	/**< List of IceCandidatePair structures */
	MSList *valid_list;	/**< List of IceValidCandidatePair structures */
	MSList *foundations;	/**< List of IcePairFoundation structures */
	MSList *componentIDs;	/**< List of uint16_t */
	IceCheckListState state;	/**< Global state of the ICE check list */
	uint64_t ta_time;	/**< Time when the Ta timer has been processed for the last time */
	uint64_t keepalive_time;	/**< Time when the last keepalive packet has been sent for this stream */
	uint32_t foundation_generator;	/**< Autoincremented integer to generate unique foundation values */
	void (*success_cb)(void *stream, struct _IceCheckList *cl);	/**< Callback function called when ICE processing finishes successfully for the check list */
	void *stream_ptr;	/**< Pointer to the media stream corresponding to the check list, to be used as an argument in the success callback */
} IceCheckList;


typedef void (*ice_check_list_success_cb)(void *stream, IceCheckList *cl);


#ifdef __cplusplus
extern "C"{
#endif

/**
 * Allocate a new ICE session.
 *
 * @return Pointer to the allocated session
 *
 * This must be performed for each media session that is to use ICE.
 */
MS2_PUBLIC IceSession * ice_session_new(void);

/**
 * Destroy a previously allocated ICE session.
 *
 * @param session The session to destroy.
 *
 * To be used when a media session using ICE is tore down.
 */
MS2_PUBLIC void ice_session_destroy(IceSession *session);

/**
 * Allocate a new ICE check list.
 *
 * @return Pointer to the allocated check list
 *
 * A check list must be allocated for each media stream of a media session and be added to an ICE session using the ice_session_add_check_list() function.
 */
MS2_PUBLIC IceCheckList * ice_check_list_new(void);

/**
 * Register the callback function to be called when the processing of the check list is successful
 *
 * @param success_cb Pointer to a callback function to be called when the processing of the check list is successful
 * @param stream_ptr Pointer to the media stream to be passed as a parameter to the callback function
 */
MS2_PUBLIC void ice_check_list_register_success_cb(IceCheckList *cl, ice_check_list_success_cb success_cb, void *stream_ptr);

/**
 * Destroy a previously allocated ICE check list.
 *
 * @param cl The check list to destroy
 */
MS2_PUBLIC void ice_check_list_destroy(IceCheckList *cl);

/**
 * Get the nth check list of an ICE session.
 *
 * @param session A pointer to a session
 * @param n The number of the check list to access
 * @return A pointer to the nth check list of the session if it exists, NULL otherwise
 */
MS2_PUBLIC IceCheckList *ice_session_check_list(const IceSession *session, unsigned int n);

/**
 * Get the local username fragment of an ICE session.
 *
 * @param session A pointer to a session
 * @return A pointer to the local username fragment of the session
 */
MS2_PUBLIC const char * ice_session_local_ufrag(const IceSession *session);

/**
 * Get the local password of an ICE session.
 *
 * @param session A pointer to a session
 * @return A pointer to the local password of the session
 */
MS2_PUBLIC const char * ice_session_local_pwd(const IceSession *session);

/**
 * Get the remote username fragment of an ICE session.
 *
 * @param session A pointer to a session
 * @return A pointer to the remote username fragment of the session
 */
MS2_PUBLIC const char * ice_session_remote_ufrag(const IceSession *session);

/**
 * Get the remote password of an ICE session.
 *
 * @param session A pointer to a session
 * @return A pointer to the remote password of the session
 */
MS2_PUBLIC const char * ice_session_remote_pwd(const IceSession *session);

/**
 * Set the role of the agent for an ICE session.
 *
 * @param session The session for which to set the role
 * @param role The role to set the session to
 */
MS2_PUBLIC void ice_session_set_role(IceSession *session, IceRole role);

/**
 * Set the local credentials of an ICE session.
 *
 * This function SHOULD not be used. However, it is used by mediastream for testing purpose to
 * apply the same credentials for local and remote agents because the SDP exchange is bypassed.
 */
void ice_session_set_local_credentials(IceSession *session, const char *ufrag, const char *pwd);

/**
 * Set the remote credentials of an ICE session.
 *
 * @param session A pointer to a session
 * @param ufrag The remote username fragment
 * @param pwd The remote password
 *
 * This function is to be called once the remote credentials have been received via SDP.
 */
MS2_PUBLIC void ice_session_set_remote_credentials(IceSession *session, const char *ufrag, const char *pwd);

/**
 * Define the maximum number of connectivity checks that will be performed by the agent.
 *
 * @param session A pointer to a session
 * @param max_connectivity_checks The maximum number of connectivity checks to perform
 *
 * This function is to be called just after the creation of the session, before any connectivity check is performed.
 * The default number of connectivity checks is 100.
 */
MS2_PUBLIC void ice_session_set_max_connectivity_checks(IceSession *session, uint8_t max_connectivity_checks);

/**
 * Define the timeout between each keepalive packet in seconds.
 *
 * @param session A pointer to a session
 * @param timeout The duration of the keepalive timeout in seconds
 *
 * The default keepalive timeout is set to 15 seconds.
 */
MS2_PUBLIC void ice_session_set_keepalive_timeout(IceSession *session, uint8_t timeout);

/**
 * Add an ICE check list to an ICE session.
 *
 * @param session The session that is assigned the check list
 * @param cl The check list to assign to the session
 */
MS2_PUBLIC void ice_session_add_check_list(IceSession *session, IceCheckList *cl);

/**
 * Get the state of an ICE check list.
 *
 * @param cl A pointer to a check list
 * @return The check list state
 */
MS2_PUBLIC IceCheckListState ice_check_list_state(const IceCheckList *cl);

/**
 * Get the local username fragment of an ICE check list.
 *
 * @param cl A pointer to a check list
 * @return A pointer to the local username fragment of the check list
 */
MS2_PUBLIC const char * ice_check_list_local_ufrag(const IceCheckList *cl);

/**
 * Get the local password of an ICE check list.
 *
 * @param cl A pointer to a check list
 * @return A pointer to the local password of the check list
 */
MS2_PUBLIC const char * ice_check_list_local_pwd(const IceCheckList *cl);

/**
 * Get the remote username fragment of an ICE check list.
 *
 * @param cl A pointer to a check list
 * @return A pointer to the remote username fragment of the check list
 */
MS2_PUBLIC const char * ice_check_list_remote_ufrag(const IceCheckList *cl);

/**
 * Get the remote password of an ICE check list.
 *
 * @param cl A pointer to a check list
 * @return A pointer to the remote password of the check list
 */
MS2_PUBLIC const char * ice_check_list_remote_pwd(const IceCheckList *cl);

/**
 * Set the remote credentials of an ICE check list.
 *
 * @param cl A pointer to a check list
 * @param ufrag The remote username fragment
 * @param pwd The remote password
 *
 * This function is to be called once the remote credentials have been received via SDP.
 */
MS2_PUBLIC void ice_check_list_set_remote_credentials(IceCheckList *cl, const char *ufrag, const char *pwd);

/**
 * Get the default local candidate for an ICE check list.
 *
 * @param cl A pointer to a check list
 * @return A pointer to the default local candidate for the check list if found, NULL otherwise
 */
MS2_PUBLIC const IceCandidate * ice_check_list_default_local_candidate(const IceCheckList *cl);

/**
 * Get the candidate type as a string.
 *
 * @param candidate A pointer to a candidate
 * @return A pointer to the candidate type as a string
 */
MS2_PUBLIC const char * ice_candidate_type(const IceCandidate *candidate);

/**
 * Add a local candidate to an ICE check list.
 *
 * This function is not to be used directly. The ice_session_gather_candidates() function SHOULD be used instead.
 * However, it is used by mediastream for testing purpose since it does not use gathering.
 */
MS2_PUBLIC IceCandidate * ice_add_local_candidate(IceCheckList *cl, const char *type, const char *ip, int port, uint16_t componentID, IceCandidate *base);

/**
 * Add a remote candidate to an ICE check list.
 *
 * @param cl A pointer to a check list
 * @param type The type of the remote candidate to add as a string (must be one of: "host", "srflx", "prflx" or "relay")
 * @param ip The IP address of the remote candidate as a string (eg. 192.168.0.10)
 * @param port The port of the remote candidate
 * @param componentID The component ID of the remote candidate (usually 1 for RTP and 2 for RTCP)
 * @param priority The priority of the remote candidate
 * @param foundation The foundation of the remote candidate
 *
 * This function is to be called once the remote candidate list has been received via SDP.
 */
MS2_PUBLIC IceCandidate * ice_add_remote_candidate(IceCheckList *cl, const char *type, const char *ip, int port, uint16_t componentID, uint32_t priority, const char * const foundation);

/**
 * Set the base for the local server reflexive candidates of an ICE session.
 *
 * This function SHOULD not be used. However, it is used by mediastream for testing purpose to
 * work around the fact that it does not use candidates gathering.
 * It is to be called automatically when the gathering process finishes.
 */
void ice_session_set_base_for_srflx_candidates(IceSession *session);

/**
 * Compute the foundations of the local candidates of an ICE session.
 *
 * @param session A pointer to a session
 *
 * This function is to be called at the end of the local candidates gathering process, before sending
 * the SDP to the remote agent.
 */
MS2_PUBLIC void ice_session_compute_candidates_foundations(IceSession *session);

/**
 * Choose the default candidates of an ICE session.
 *
 * @param session A pointer to a session
 *
 * This function is to be called at the end of the local candidates gathering process, before sending
 * the SDP to the remote agent.
 */
MS2_PUBLIC void ice_session_choose_default_candidates(IceSession *session);

/**
 * Pair the local and the remote candidates for an ICE session.
 *
 * @param session A pointer to a session
 */
MS2_PUBLIC void ice_session_pair_candidates(IceSession *session);

/**
 * Core ICE check list processing.
 *
 * This function is called from the audiostream or the videostream and is NOT to be called by the user.
 */
void ice_check_list_process(IceCheckList* cl, const RtpSession* rtp_session);

/**
 * Handle a STUN packet that has been received.
 *
 * This function is called from the audiostream or the videostream and is NOT to be called by the user.
 */
void ice_handle_stun_packet(IceCheckList* cl, const RtpSession* rtp_session, const OrtpEventData* evt_data);

/**
 * Get the remote address, RTP port and RTCP port to use to send the stream once the ICE process has finished successfully.
 *
 * @param cl A pointer to a check list
 * @param addr A pointer to the buffer to use to store the remote address
 * @param addr_len The size of the buffer to use to store the remote address
 * @param rtp_port A pointer to the location to store the RTP port to
 * @param rtcp_port A pointer to the location to store the RTCP port to
 *
 * This function will usually be called from within the success callback defined while creating the ICE check list with ice_check_list_new().
 */
MS2_PUBLIC void ice_get_remote_addr_and_ports_from_valid_pairs(const IceCheckList* cl, char* addr, int addr_len, int* rtp_port, int* rtcp_port);

/**
 * Print the route used to send the stream if the ICE process has finished successfully.
 *
 * @param cl A pointer to a check list
 * @param message A message to print before the route
 */
MS2_PUBLIC void ice_check_list_print_route(const IceCheckList *cl, const char *message);

/**
 * Dump an ICE session in the traces (debug function).
 */
MS2_PUBLIC void ice_dump_session(const IceSession *session);

/**
 * Dump the candidates of an ICE check list in the traces (debug function).
 */
MS2_PUBLIC void ice_dump_candidates(const IceCheckList *cl);

/**
 * Dump the candidate pairs of an ICE check list in the traces (debug function).
 */
MS2_PUBLIC void ice_dump_candidate_pairs(const IceCheckList *cl);

/**
 * Dump the valid list of an ICE check list in the traces (debug function).
 */
MS2_PUBLIC void ice_dump_valid_list(const IceCheckList *cl);

/**
 * Dump the list of candidate pair foundations of an ICE check list in the traces (debug function).
 */
MS2_PUBLIC void ice_dump_candidate_pairs_foundations(const IceCheckList *cl);

/**
 * Dump the list of component IDs of an ICE check list in the traces (debug function).
 */
MS2_PUBLIC void ice_dump_componentIDs(const IceCheckList *cl);

/**
 * Dump an ICE check list in the traces (debug function).
 */
MS2_PUBLIC void ice_dump_check_list(const IceCheckList *cl);

/**
 * Dump the triggered checks queue of an ICE check list in the traces (debug function).
 */
MS2_PUBLIC void ice_dump_triggered_checks_queue(const IceCheckList *cl);

#ifdef __cplusplus
}
#endif

#endif
