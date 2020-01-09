/* Do not edit: automatically built by gen_msg.awk. */

#include "db_config.h"
#include "db_int.h"
#pragma hdrstop
#include "dbinc/db_swap.h"
/*
 * PUBLIC: void __repmgr_handshake_marshal(ENV *, __repmgr_handshake_args *, uint8 *);
 */
void __repmgr_handshake_marshal(ENV *env, __repmgr_handshake_args * argp, uint8 * bp)
{
	DB_HTONS_COPYOUT(env, bp, argp->port);
	DB_HTONS_COPYOUT(env, bp, argp->alignment);
	DB_HTONL_COPYOUT(env, bp, argp->ack_policy);
	DB_HTONL_COPYOUT(env, bp, argp->flags);
}
/*
 * PUBLIC: int __repmgr_handshake_unmarshal(ENV *, __repmgr_handshake_args *, uint8 *, size_t, uint8 **);
 */
int __repmgr_handshake_unmarshal(ENV *env, __repmgr_handshake_args * argp, uint8 * bp, size_t max, uint8 ** nextp)
{
	if(max < __REPMGR_HANDSHAKE_SIZE)
		goto too_few;
	DB_NTOHS_COPYIN(env, argp->port, bp);
	DB_NTOHS_COPYIN(env, argp->alignment, bp);
	DB_NTOHL_COPYIN(env, argp->ack_policy, bp);
	DB_NTOHL_COPYIN(env, argp->flags, bp);
	if(nextp != NULL)
		*nextp = bp;
	return 0;
too_few:
	__db_errx(env, DB_STR("3675", "Not enough input bytes to fill a __repmgr_handshake message"));
	return EINVAL;
}
/*
 * PUBLIC: void __repmgr_v3handshake_marshal(ENV *, __repmgr_v3handshake_args *, uint8 *);
 */
void __repmgr_v3handshake_marshal(ENV *env, __repmgr_v3handshake_args * argp, uint8 * bp)
{
	DB_HTONS_COPYOUT(env, bp, argp->port);
	DB_HTONL_COPYOUT(env, bp, argp->priority);
	DB_HTONL_COPYOUT(env, bp, argp->flags);
}
/*
 * PUBLIC: int __repmgr_v3handshake_unmarshal(ENV *, __repmgr_v3handshake_args *, uint8 *, size_t, uint8 **);
 */
int __repmgr_v3handshake_unmarshal(ENV *env, __repmgr_v3handshake_args * argp, uint8 * bp, size_t max, uint8 ** nextp)
{
	if(max < __REPMGR_V3HANDSHAKE_SIZE)
		goto too_few;
	DB_NTOHS_COPYIN(env, argp->port, bp);
	DB_NTOHL_COPYIN(env, argp->priority, bp);
	DB_NTOHL_COPYIN(env, argp->flags, bp);
	if(nextp != NULL)
		*nextp = bp;
	return 0;
too_few:
	__db_errx(env, DB_STR("3675", "Not enough input bytes to fill a __repmgr_v3handshake message"));
	return EINVAL;
}
/*
 * PUBLIC: void __repmgr_v2handshake_marshal(ENV *, __repmgr_v2handshake_args *, uint8 *);
 */
void __repmgr_v2handshake_marshal(ENV *env, __repmgr_v2handshake_args * argp, uint8 * bp)
{
	DB_HTONS_COPYOUT(env, bp, argp->port);
	DB_HTONL_COPYOUT(env, bp, argp->priority);
}
/*
 * PUBLIC: int __repmgr_v2handshake_unmarshal(ENV *, __repmgr_v2handshake_args *, uint8 *, size_t, uint8 **);
 */
int __repmgr_v2handshake_unmarshal(ENV *env, __repmgr_v2handshake_args * argp, uint8 * bp, size_t max, uint8 ** nextp)
{
	if(max < __REPMGR_V2HANDSHAKE_SIZE)
		goto too_few;
	DB_NTOHS_COPYIN(env, argp->port, bp);
	DB_NTOHL_COPYIN(env, argp->priority, bp);
	if(nextp != NULL)
		*nextp = bp;
	return 0;
too_few:
	__db_errx(env, DB_STR("3675", "Not enough input bytes to fill a __repmgr_v2handshake message"));
	return EINVAL;
}
/*
 * PUBLIC: void __repmgr_parm_refresh_marshal(ENV *, __repmgr_parm_refresh_args *, uint8 *);
 */
void __repmgr_parm_refresh_marshal(ENV *env, __repmgr_parm_refresh_args * argp, uint8 * bp)
{
	DB_HTONL_COPYOUT(env, bp, argp->ack_policy);
	DB_HTONL_COPYOUT(env, bp, argp->flags);
}

/*
 * PUBLIC: int __repmgr_parm_refresh_unmarshal(ENV *, __repmgr_parm_refresh_args *, uint8 *, size_t, uint8 **);
 */
int __repmgr_parm_refresh_unmarshal(ENV *env, __repmgr_parm_refresh_args * argp, uint8 * bp, size_t max, uint8 ** nextp)
{
	if(max < __REPMGR_PARM_REFRESH_SIZE)
		goto too_few;
	DB_NTOHL_COPYIN(env, argp->ack_policy, bp);
	DB_NTOHL_COPYIN(env, argp->flags, bp);
	if(nextp != NULL)
		*nextp = bp;
	return 0;
too_few:
	__db_errx(env, DB_STR("3675", "Not enough input bytes to fill a __repmgr_parm_refresh message"));
	return EINVAL;
}
/*
 * PUBLIC: void __repmgr_permlsn_marshal(ENV *, __repmgr_permlsn_args *, uint8 *);
 */
void __repmgr_permlsn_marshal(ENV *env, __repmgr_permlsn_args * argp, uint8 * bp)
{
	DB_HTONL_COPYOUT(env, bp, argp->generation);
	DB_HTONL_COPYOUT(env, bp, argp->lsn.file);
	DB_HTONL_COPYOUT(env, bp, argp->lsn.offset);
}
/*
 * PUBLIC: int __repmgr_permlsn_unmarshal(ENV *, __repmgr_permlsn_args *, uint8 *, size_t, uint8 **);
 */
int __repmgr_permlsn_unmarshal(ENV *env, __repmgr_permlsn_args * argp, uint8 * bp, size_t max, uint8 ** nextp)
{
	if(max < __REPMGR_PERMLSN_SIZE)
		goto too_few;
	DB_NTOHL_COPYIN(env, argp->generation, bp);
	DB_NTOHL_COPYIN(env, argp->lsn.file, bp);
	DB_NTOHL_COPYIN(env, argp->lsn.offset, bp);
	if(nextp != NULL)
		*nextp = bp;
	return 0;
too_few:
	__db_errx(env, DB_STR("3675", "Not enough input bytes to fill a __repmgr_permlsn message"));
	return EINVAL;
}
/*
 * PUBLIC: void __repmgr_version_proposal_marshal(ENV *, __repmgr_version_proposal_args *, uint8 *));
 */
void __repmgr_version_proposal_marshal(ENV *env, __repmgr_version_proposal_args * argp, uint8 * bp)
{
	DB_HTONL_COPYOUT(env, bp, argp->min);
	DB_HTONL_COPYOUT(env, bp, argp->max);
}
/*
 * PUBLIC: int __repmgr_version_proposal_unmarshal(ENV *, __repmgr_version_proposal_args *, uint8 *, size_t, uint8 **);
 */
int __repmgr_version_proposal_unmarshal(ENV *env, __repmgr_version_proposal_args * argp, uint8 * bp, size_t max, uint8 ** nextp)
{
	if(max < __REPMGR_VERSION_PROPOSAL_SIZE)
		goto too_few;
	DB_NTOHL_COPYIN(env, argp->min, bp);
	DB_NTOHL_COPYIN(env, argp->max, bp);

	if(nextp != NULL)
		*nextp = bp;
	return 0;
too_few:
	__db_errx(env, DB_STR("3675", "Not enough input bytes to fill a __repmgr_version_proposal message"));
	return EINVAL;
}
/*
 * PUBLIC: void __repmgr_version_confirmation_marshal(ENV *, __repmgr_version_confirmation_args *, uint8 *);
 */
void __repmgr_version_confirmation_marshal(ENV *env, __repmgr_version_confirmation_args * argp, uint8 * bp)
{
	DB_HTONL_COPYOUT(env, bp, argp->version);
}
/*
 * PUBLIC: int __repmgr_version_confirmation_unmarshal(ENV *, __repmgr_version_confirmation_args *, uint8 *, size_t, uint8 **);
 */
int __repmgr_version_confirmation_unmarshal(ENV *env, __repmgr_version_confirmation_args * argp, uint8 * bp, size_t max, uint8 ** nextp)
{
	if(max < __REPMGR_VERSION_CONFIRMATION_SIZE)
		goto too_few;
	DB_NTOHL_COPYIN(env, argp->version, bp);

	if(nextp != NULL)
		*nextp = bp;
	return 0;
too_few:
	__db_errx(env, DB_STR("3675", "Not enough input bytes to fill a __repmgr_version_confirmation message"));
	return EINVAL;
}
/*
 * PUBLIC: void __repmgr_msg_hdr_marshal(ENV *, __repmgr_msg_hdr_args *, uint8 *);
 */
void __repmgr_msg_hdr_marshal(ENV *env, __repmgr_msg_hdr_args * argp, uint8 * bp)
{
	*bp++ = argp->type;
	DB_HTONL_COPYOUT(env, bp, argp->word1);
	DB_HTONL_COPYOUT(env, bp, argp->word2);
}
/*
 * PUBLIC: int __repmgr_msg_hdr_unmarshal(ENV *, __repmgr_msg_hdr_args *, uint8 *, size_t, uint8 **);
 */
int __repmgr_msg_hdr_unmarshal(ENV *env, __repmgr_msg_hdr_args * argp, uint8 * bp, size_t max, uint8 ** nextp)
{
	if(max < __REPMGR_MSG_HDR_SIZE)
		goto too_few;
	argp->type = *bp++;
	DB_NTOHL_COPYIN(env, argp->word1, bp);
	DB_NTOHL_COPYIN(env, argp->word2, bp);

	if(nextp != NULL)
		*nextp = bp;
	return 0;
too_few:
	__db_errx(env, DB_STR("3675", "Not enough input bytes to fill a __repmgr_msg_hdr message"));
	return EINVAL;
}
/*
 * PUBLIC: void __repmgr_msg_metadata_marshal(ENV *, __repmgr_msg_metadata_args *, uint8 *);
 */
void __repmgr_msg_metadata_marshal(ENV *env, __repmgr_msg_metadata_args * argp, uint8 * bp)
{
	DB_HTONL_COPYOUT(env, bp, argp->tag);
	DB_HTONL_COPYOUT(env, bp, argp->limit);
	DB_HTONL_COPYOUT(env, bp, argp->flags);
}
/*
 * PUBLIC: int __repmgr_msg_metadata_unmarshal(ENV *, __repmgr_msg_metadata_args *, uint8 *, size_t, uint8 **);
 */
int __repmgr_msg_metadata_unmarshal(ENV *env, __repmgr_msg_metadata_args * argp, uint8 * bp, size_t max, uint8 ** nextp)
{
	if(max < __REPMGR_MSG_METADATA_SIZE)
		goto too_few;
	DB_NTOHL_COPYIN(env, argp->tag, bp);
	DB_NTOHL_COPYIN(env, argp->limit, bp);
	DB_NTOHL_COPYIN(env, argp->flags, bp);
	if(nextp != NULL)
		*nextp = bp;
	return 0;
too_few:
	__db_errx(env, DB_STR("3675", "Not enough input bytes to fill a __repmgr_msg_metadata message"));
	return EINVAL;
}

/*
 * PUBLIC: int __repmgr_membership_key_marshal(ENV *, __repmgr_membership_key_args *, uint8 *, size_t, size_t *);
 */
int __repmgr_membership_key_marshal(ENV *env, __repmgr_membership_key_args * argp, uint8 * bp, size_t max, size_t * lenp)
{
	uint8 * start;
	if(max < __REPMGR_MEMBERSHIP_KEY_SIZE + (size_t)argp->host.size)
		return (ENOMEM);
	start = bp;
	DB_HTONL_COPYOUT(env, bp, argp->host.size);
	if(argp->host.size > 0) {
		memcpy(bp, argp->host.data, argp->host.size);
		bp += argp->host.size;
	}
	DB_HTONS_COPYOUT(env, bp, argp->port);

	*lenp = (size_t)(bp - start);
	return 0;
}
/*
 * PUBLIC: int __repmgr_membership_key_unmarshal(ENV *, __repmgr_membership_key_args *, uint8 *, size_t, uint8 **);
 */
int __repmgr_membership_key_unmarshal(ENV *env, __repmgr_membership_key_args * argp, uint8 * bp, size_t max, uint8 ** nextp)
{
	size_t needed = __REPMGR_MEMBERSHIP_KEY_SIZE;
	if(max < needed)
		goto too_few;
	DB_NTOHL_COPYIN(env, argp->host.size, bp);
	if(argp->host.size == 0)
		argp->host.data = NULL;
	else
		argp->host.data = bp;
	needed += (size_t)argp->host.size;
	if(max < needed)
		goto too_few;
	bp += argp->host.size;
	DB_NTOHS_COPYIN(env, argp->port, bp);

	if(nextp != NULL)
		*nextp = bp;
	return 0;
too_few:
	__db_errx(env, DB_STR("3675", "Not enough input bytes to fill a __repmgr_membership_key message"));
	return EINVAL;
}
/*
 * PUBLIC: void __repmgr_membership_data_marshal(ENV *, __repmgr_membership_data_args *, uint8 *);
 */
void __repmgr_membership_data_marshal(ENV *env, __repmgr_membership_data_args * argp, uint8 * bp)
{
	DB_HTONL_COPYOUT(env, bp, argp->status);
	DB_HTONL_COPYOUT(env, bp, argp->flags);
}
/*
 * PUBLIC: int __repmgr_membership_data_unmarshal(ENV *, __repmgr_membership_data_args *, uint8 *, size_t, uint8 **);
 */
int __repmgr_membership_data_unmarshal(ENV *env, __repmgr_membership_data_args * argp, uint8 * bp, size_t max, uint8 ** nextp)
{
	if(max < __REPMGR_MEMBERSHIP_DATA_SIZE)
		goto too_few;
	DB_NTOHL_COPYIN(env, argp->status, bp);
	DB_NTOHL_COPYIN(env, argp->flags, bp);
	if(nextp != NULL)
		*nextp = bp;
	return 0;
too_few:
	__db_errx(env, DB_STR("3675", "Not enough input bytes to fill a __repmgr_membership_data message"));
	return EINVAL;
}
/*
 * PUBLIC: void __repmgr_v4membership_data_marshal(ENV *, __repmgr_v4membership_data_args *, uint8 *);
 */
void __repmgr_v4membership_data_marshal(ENV *env, __repmgr_v4membership_data_args * argp, uint8 * bp)
{
	DB_HTONL_COPYOUT(env, bp, argp->flags);
}
/*
 * PUBLIC: int __repmgr_v4membership_data_unmarshal(ENV *, __repmgr_v4membership_data_args *, uint8 *, size_t, uint8 **);
 */
int __repmgr_v4membership_data_unmarshal(ENV *env, __repmgr_v4membership_data_args * argp, uint8 * bp, size_t max, uint8 ** nextp)
{
	if(max < __REPMGR_V4MEMBERSHIP_DATA_SIZE)
		goto too_few;
	DB_NTOHL_COPYIN(env, argp->flags, bp);
	if(nextp != NULL)
		*nextp = bp;
	return 0;
too_few:
	__db_errx(env, DB_STR("3675", "Not enough input bytes to fill a __repmgr_v4membership_data message"));
	return EINVAL;
}
/*
 * PUBLIC: void __repmgr_member_metadata_marshal(ENV *, __repmgr_member_metadata_args *, uint8 *);
 */
void __repmgr_member_metadata_marshal(ENV *env, __repmgr_member_metadata_args * argp, uint8 * bp)
{
	DB_HTONL_COPYOUT(env, bp, argp->format);
	DB_HTONL_COPYOUT(env, bp, argp->version);
}
/*
 * PUBLIC: int __repmgr_member_metadata_unmarshal(ENV *, __repmgr_member_metadata_args *, uint8 *, size_t, uint8 **);
 */
int __repmgr_member_metadata_unmarshal(ENV *env, __repmgr_member_metadata_args * argp, uint8 * bp, size_t max, uint8 ** nextp)
{
	if(max < __REPMGR_MEMBER_METADATA_SIZE)
		goto too_few;
	DB_NTOHL_COPYIN(env, argp->format, bp);
	DB_NTOHL_COPYIN(env, argp->version, bp);
	if(nextp != NULL)
		*nextp = bp;
	return 0;
too_few:
	__db_errx(env, DB_STR("3675", "Not enough input bytes to fill a __repmgr_member_metadata message"));
	return EINVAL;
}
/*
 * PUBLIC: int __repmgr_gm_fwd_marshal(ENV *, __repmgr_gm_fwd_args *, uint8 *, size_t, size_t *);
 */
int __repmgr_gm_fwd_marshal(ENV *env, __repmgr_gm_fwd_args * argp, uint8 * bp, size_t max, size_t * lenp)
{
	uint8 * start;
	if(max < __REPMGR_GM_FWD_SIZE + (size_t)argp->host.size)
		return (ENOMEM);
	start = bp;
	DB_HTONL_COPYOUT(env, bp, argp->host.size);
	if(argp->host.size > 0) {
		memcpy(bp, argp->host.data, argp->host.size);
		bp += argp->host.size;
	}
	DB_HTONS_COPYOUT(env, bp, argp->port);
	DB_HTONL_COPYOUT(env, bp, argp->gen);
	*lenp = (size_t)(bp - start);
	return 0;
}
/*
 * PUBLIC: int __repmgr_gm_fwd_unmarshal(ENV *, __repmgr_gm_fwd_args *, uint8 *, size_t, uint8 **);
 */
int __repmgr_gm_fwd_unmarshal(ENV *env, __repmgr_gm_fwd_args * argp, uint8 * bp, size_t max, uint8 ** nextp)
{
	size_t needed = __REPMGR_GM_FWD_SIZE;
	if(max < needed)
		goto too_few;
	DB_NTOHL_COPYIN(env, argp->host.size, bp);
	if(argp->host.size == 0)
		argp->host.data = NULL;
	else
		argp->host.data = bp;
	needed += (size_t)argp->host.size;
	if(max < needed)
		goto too_few;
	bp += argp->host.size;
	DB_NTOHS_COPYIN(env, argp->port, bp);
	DB_NTOHL_COPYIN(env, argp->gen, bp);
	if(nextp != NULL)
		*nextp = bp;
	return 0;
too_few:
	__db_errx(env, DB_STR("3675", "Not enough input bytes to fill a __repmgr_gm_fwd message"));
	return EINVAL;
}
/*
 * PUBLIC: void __repmgr_membr_vers_marshal(ENV *, __repmgr_membr_vers_args *, uint8 *);
 */
void __repmgr_membr_vers_marshal(ENV *env, __repmgr_membr_vers_args * argp, uint8 * bp)
{
	DB_HTONL_COPYOUT(env, bp, argp->version);
	DB_HTONL_COPYOUT(env, bp, argp->gen);
}
/*
 * PUBLIC: int __repmgr_membr_vers_unmarshal(ENV *, __repmgr_membr_vers_args *, uint8 *, size_t, uint8 **);
 */
int __repmgr_membr_vers_unmarshal(ENV *env, __repmgr_membr_vers_args * argp, uint8 * bp, size_t max, uint8 ** nextp)
{
	if(max < __REPMGR_MEMBR_VERS_SIZE)
		goto too_few;
	DB_NTOHL_COPYIN(env, argp->version, bp);
	DB_NTOHL_COPYIN(env, argp->gen, bp);
	if(nextp != NULL)
		*nextp = bp;
	return 0;
too_few:
	__db_errx(env, DB_STR("3675", "Not enough input bytes to fill a __repmgr_membr_vers message"));
	return EINVAL;
}
/*
 * PUBLIC: int __repmgr_site_info_marshal(ENV *, __repmgr_site_info_args *, uint8 *, size_t, size_t *);
 */
int __repmgr_site_info_marshal(ENV *env, __repmgr_site_info_args * argp, uint8 * bp, size_t max, size_t * lenp)
{
	uint8 * start;
	if(max < __REPMGR_SITE_INFO_SIZE + (size_t)argp->host.size)
		return (ENOMEM);
	start = bp;
	DB_HTONL_COPYOUT(env, bp, argp->host.size);
	if(argp->host.size > 0) {
		memcpy(bp, argp->host.data, argp->host.size);
		bp += argp->host.size;
	}
	DB_HTONS_COPYOUT(env, bp, argp->port);
	DB_HTONL_COPYOUT(env, bp, argp->status);
	DB_HTONL_COPYOUT(env, bp, argp->flags);

	*lenp = (size_t)(bp - start);
	return 0;
}
/*
 * PUBLIC: int __repmgr_site_info_unmarshal(ENV *, __repmgr_site_info_args *, uint8 *, size_t, uint8 **);
 */
int __repmgr_site_info_unmarshal(ENV *env, __repmgr_site_info_args * argp, uint8 * bp, size_t max, uint8 ** nextp)
{
	size_t needed = __REPMGR_SITE_INFO_SIZE;
	if(max < needed)
		goto too_few;
	DB_NTOHL_COPYIN(env, argp->host.size, bp);
	if(argp->host.size == 0)
		argp->host.data = NULL;
	else
		argp->host.data = bp;
	needed += (size_t)argp->host.size;
	if(max < needed)
		goto too_few;
	bp += argp->host.size;
	DB_NTOHS_COPYIN(env, argp->port, bp);
	DB_NTOHL_COPYIN(env, argp->status, bp);
	DB_NTOHL_COPYIN(env, argp->flags, bp);
	if(nextp != NULL)
		*nextp = bp;
	return 0;
too_few:
	__db_errx(env, DB_STR("3675", "Not enough input bytes to fill a __repmgr_site_info message"));
	return EINVAL;
}
/*
 * PUBLIC: int __repmgr_v4site_info_marshal(ENV *, __repmgr_v4site_info_args *, uint8 *, size_t, size_t *);
 */
int __repmgr_v4site_info_marshal(ENV *env, __repmgr_v4site_info_args * argp, uint8 * bp, size_t max, size_t * lenp)
{
	uint8 * start;
	if(max < __REPMGR_V4SITE_INFO_SIZE + (size_t)argp->host.size)
		return (ENOMEM);
	start = bp;
	DB_HTONL_COPYOUT(env, bp, argp->host.size);
	if(argp->host.size > 0) {
		memcpy(bp, argp->host.data, argp->host.size);
		bp += argp->host.size;
	}
	DB_HTONS_COPYOUT(env, bp, argp->port);
	DB_HTONL_COPYOUT(env, bp, argp->flags);
	*lenp = (size_t)(bp - start);
	return 0;
}
/*
 * PUBLIC: int __repmgr_v4site_info_unmarshal(ENV *, __repmgr_v4site_info_args *, uint8 *, size_t, uint8 **);
 */
int __repmgr_v4site_info_unmarshal(ENV *env, __repmgr_v4site_info_args * argp, uint8 * bp, size_t max, uint8 ** nextp)
{
	size_t needed = __REPMGR_V4SITE_INFO_SIZE;
	if(max < needed)
		goto too_few;
	DB_NTOHL_COPYIN(env, argp->host.size, bp);
	if(argp->host.size == 0)
		argp->host.data = NULL;
	else
		argp->host.data = bp;
	needed += (size_t)argp->host.size;
	if(max < needed)
		goto too_few;
	bp += argp->host.size;
	DB_NTOHS_COPYIN(env, argp->port, bp);
	DB_NTOHL_COPYIN(env, argp->flags, bp);

	if(nextp != NULL)
		*nextp = bp;
	return 0;
too_few:
	__db_errx(env, DB_STR("3675", "Not enough input bytes to fill a __repmgr_v4site_info message"));
	return EINVAL;
}
/*
 * PUBLIC: void __repmgr_connect_reject_marshal(ENV *, __repmgr_connect_reject_args *, uint8 *);
 */
void __repmgr_connect_reject_marshal(ENV *env, __repmgr_connect_reject_args * argp, uint8 * bp)
{
	DB_HTONL_COPYOUT(env, bp, argp->version);
	DB_HTONL_COPYOUT(env, bp, argp->gen);
	DB_HTONL_COPYOUT(env, bp, argp->status);
}
/*
 * PUBLIC: int __repmgr_connect_reject_unmarshal(ENV *, __repmgr_connect_reject_args *, uint8 *, size_t, uint8 **);
 */
int __repmgr_connect_reject_unmarshal(ENV *env, __repmgr_connect_reject_args * argp, uint8 * bp, size_t max, uint8 ** nextp)
{
	if(max < __REPMGR_CONNECT_REJECT_SIZE)
		goto too_few;
	DB_NTOHL_COPYIN(env, argp->version, bp);
	DB_NTOHL_COPYIN(env, argp->gen, bp);
	DB_NTOHL_COPYIN(env, argp->status, bp);

	if(nextp != NULL)
		*nextp = bp;
	return 0;
too_few:
	__db_errx(env, DB_STR("3675", "Not enough input bytes to fill a __repmgr_connect_reject message"));
	return EINVAL;
}
/*
 * PUBLIC: void __repmgr_v4connect_reject_marshal(ENV *, __repmgr_v4connect_reject_args *, uint8 *);
 */
void __repmgr_v4connect_reject_marshal(ENV *env, __repmgr_v4connect_reject_args * argp, uint8 * bp)
{
	DB_HTONL_COPYOUT(env, bp, argp->version);
	DB_HTONL_COPYOUT(env, bp, argp->gen);
}
/*
 * PUBLIC: int __repmgr_v4connect_reject_unmarshal(ENV *, __repmgr_v4connect_reject_args *, uint8 *, size_t, uint8 **);
 */
int __repmgr_v4connect_reject_unmarshal(ENV *env, __repmgr_v4connect_reject_args * argp, uint8 * bp, size_t max, uint8 ** nextp)
{
	if(max < __REPMGR_V4CONNECT_REJECT_SIZE)
		goto too_few;
	DB_NTOHL_COPYIN(env, argp->version, bp);
	DB_NTOHL_COPYIN(env, argp->gen, bp);
	if(nextp != NULL)
		*nextp = bp;
	return 0;
too_few:
	__db_errx(env, DB_STR("3675", "Not enough input bytes to fill a __repmgr_v4connect_reject message"));
	return EINVAL;
}
/*
 * PUBLIC: void __repmgr_lsnhist_match_marshal(ENV *, __repmgr_lsnhist_match_args *, uint8 *);
 */
void __repmgr_lsnhist_match_marshal(ENV *env, __repmgr_lsnhist_match_args * argp, uint8 * bp)
{
	DB_HTONL_COPYOUT(env, bp, argp->lsn.file);
	DB_HTONL_COPYOUT(env, bp, argp->lsn.offset);
	DB_HTONL_COPYOUT(env, bp, argp->hist_sec);
	DB_HTONL_COPYOUT(env, bp, argp->hist_nsec);
	DB_HTONL_COPYOUT(env, bp, argp->next_gen_lsn.file);
	DB_HTONL_COPYOUT(env, bp, argp->next_gen_lsn.offset);
}
/*
 * PUBLIC: int __repmgr_lsnhist_match_unmarshal(ENV *, __repmgr_lsnhist_match_args *, uint8 *, size_t, uint8 **);
 */
int __repmgr_lsnhist_match_unmarshal(ENV *env, __repmgr_lsnhist_match_args * argp, uint8 * bp, size_t max, uint8 ** nextp)
{
	if(max < __REPMGR_LSNHIST_MATCH_SIZE)
		goto too_few;
	DB_NTOHL_COPYIN(env, argp->lsn.file, bp);
	DB_NTOHL_COPYIN(env, argp->lsn.offset, bp);
	DB_NTOHL_COPYIN(env, argp->hist_sec, bp);
	DB_NTOHL_COPYIN(env, argp->hist_nsec, bp);
	DB_NTOHL_COPYIN(env, argp->next_gen_lsn.file, bp);
	DB_NTOHL_COPYIN(env, argp->next_gen_lsn.offset, bp);
	if(nextp != NULL)
		*nextp = bp;
	return 0;
too_few:
	__db_errx(env, DB_STR("3675", "Not enough input bytes to fill a __repmgr_lsnhist_match message"));
	return EINVAL;
}
