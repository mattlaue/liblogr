#ifndef __SYSLOG_H__
#define __SYSLOG_H__

#define	LOG_EMERG	0
#define	LOG_ALERT	1
#define	LOG_CRIT	2
#define	LOG_ERR		3
#define	LOG_WARNING	4
#define	LOG_NOTICE	5
#define	LOG_INFO	6
#define	LOG_DEBUG	7
#define INTERNAL_NOPRI  0x10    /* the "no priority" priority */
                                /* mark "facility" */
typedef struct _code {
        const char      *c_name;
        int             c_val;
} CODE;

#ifdef SYSLOG_NAMES
CODE prioritynames[] = {
    { "alert",      LOG_ALERT,      },
    { "crit",       LOG_CRIT,       },
    { "debug",      LOG_DEBUG,      },
    { "emerg",      LOG_EMERG,      },
    { "err",        LOG_ERR,        },
    { "error",      LOG_ERR,        },      /* DEPRECATED */
    { "info",       LOG_INFO,       },
    { "none",       INTERNAL_NOPRI, },      /* INTERNAL */
    { "notice",     LOG_NOTICE,     },
    { "panic",      LOG_EMERG,      },      /* DEPRECATED */
    { "warn",       LOG_WARNING,    },      /* DEPRECATED */
    { "warning",    LOG_WARNING,    },
    { NULL,         -1,             }
};
#endif /* SYSLOG_NAMES */
#endif /* __SYSLOG_H__ */
