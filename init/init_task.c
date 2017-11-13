#include <linux/init_task.h>
#include <linux/export.h>
#include <linux/mqueue.h>
#include <linux/sched.h>
#include <linux/sched/sysctl.h>
#include <linux/sched/rt.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/mm.h>

#include <asm/pgtable.h>
#include <asm/uaccess.h>

static struct signal_struct init_signals = INIT_SIGNALS(init_signals);
static struct sighand_struct init_sighand = INIT_SIGHAND(init_sighand);

static struct sighand_struct init_sighand = { .count		= ATOMIC_INIT(1),
	.action		= { { { .sa_handler = SIG_DFL, } }, },
	.siglock	= __SPIN_LOCK_UNLOCKED(init_sighand.siglock),
	.signalfd_wqh	= __WAIT_QUEUE_HEAD_INITIALIZER(init_sighand.signalfd_wqh),
};

/*
 * Set up the first task table, touch at your own risk!. Base=0,
 * limit=0x1fffff (=2MB)
 */
struct task_struct init_task
#ifdef CONFIG_ARCH_TASK_STRUCT_ON_STACK
	__init_task_data
#endif
= {
#ifdef CONFIG_SCHED_PDS
#ifdef CONFIG_THREAD_INFO_IN_TASK
	.thread_info	= INIT_THREAD_INFO(init_task),
	.stack_refcount	= ATOMIC_INIT(1),
#endif
	.state		= 0,
	.stack		= init_stack,
	.usage		= ATOMIC_INIT(2),
	.flags		= PF_KTHREAD,
	.prio		= NORMAL_PRIO,
	.static_prio	= MAX_PRIO - 20,
	.normal_prio	= NORMAL_PRIO,
	.deadline	= 0, /* PDS only */
	.policy		= SCHED_NORMAL,
	.cpus_allowed	= CPU_MASK_ALL,
	.nr_cpus_allowed= NR_CPUS,
	.mm		= NULL,
	.active_mm	= &init_mm,
	.restart_block	= {
		.fn = do_no_restart_syscall,
	},
	.sl_level	= 0, /* PDS only */
	.sl_node	= SKIPLIST_NODE_INIT(init_task.sl_node), /* PDS only */
	.time_slice	= HZ, /* PDS only */
	.tasks		= LIST_HEAD_INIT(init_task.tasks),
#ifdef CONFIG_SMP
	.pushable_tasks	= PLIST_NODE_INIT(init_task.pushable_tasks, MAX_PRIO),
#endif
#ifdef CONFIG_CGROUP_SCHED
	/* PDS not supported yet
	.sched_task_group = &root_task_group,*/
#endif
	.ptraced	= LIST_HEAD_INIT(init_task.ptraced),
	.ptrace_entry	= LIST_HEAD_INIT(init_task.ptrace_entry),
	.real_parent	= &init_task,
	.parent		= &init_task,
	.children	= LIST_HEAD_INIT(init_task.children),
	.sibling	= LIST_HEAD_INIT(init_task.sibling),
	.group_leader	= &init_task,
	RCU_POINTER_INITIALIZER(real_cred, &init_cred),
	RCU_POINTER_INITIALIZER(cred, &init_cred),
	.comm		= INIT_TASK_COMM,
	.thread		= INIT_THREAD,
	.fs		= &init_fs,
	.files		= &init_files,
	.signal		= &init_signals,
	.sighand	= &init_sighand,
	.nsproxy	= &init_nsproxy,
	.pending	= {
		.list = LIST_HEAD_INIT(init_task.pending.list),
		.signal = {{0}}
	},
	.blocked	= {{0}},
	.alloc_lock	= __SPIN_LOCK_UNLOCKED(init_task.alloc_lock),
	.journal_info	= NULL,
	INIT_CPU_TIMERS(init_task)
	.pi_lock	= __RAW_SPIN_LOCK_UNLOCKED(init_task.pi_lock),
	.timer_slack_ns = 50000, /* 50 usec default slack */
	.pids = {
		[PIDTYPE_PID]  = INIT_PID_LINK(PIDTYPE_PID),
		[PIDTYPE_PGID] = INIT_PID_LINK(PIDTYPE_PGID),
		[PIDTYPE_SID]  = INIT_PID_LINK(PIDTYPE_SID),
	},
	.thread_group	= LIST_HEAD_INIT(init_task.thread_group),
	.thread_node	= LIST_HEAD_INIT(init_signals.thread_head),
#ifdef CONFIG_AUDITSYSCALL
	.loginuid	= INVALID_UID,
	.sessionid	= (unsigned int)-1,
#endif
#ifdef CONFIG_PERF_EVENTS
	.perf_event_mutex = __MUTEX_INITIALIZER(init_task.perf_event_mutex),
	.perf_event_list = LIST_HEAD_INIT(init_task.perf_event_list),
#endif
#ifdef CONFIG_PREEMPT_RCU
	.rcu_read_lock_nesting = 0,
	.rcu_read_unlock_special.s = 0,
	.rcu_node_entry = LIST_HEAD_INIT(init_task.rcu_node_entry),
	.rcu_blocked_node = NULL,
#endif
#ifdef CONFIG_TASKS_RCU
	.rcu_tasks_holdout = false,
	.rcu_tasks_holdout_list = LIST_HEAD_INIT(init_task.rcu_tasks_holdout_list),
	.rcu_tasks_idle_cpu = -1,
#endif
#ifdef CONFIG_CPUSETS
	.mems_allowed_seq = SEQCNT_ZERO(init_task.mems_allowed_seq),
#endif
#ifdef CONFIG_RT_MUTEXES
	.pi_waiters	= RB_ROOT_CACHED,
	.pi_top_task	= NULL,
#endif
	INIT_PREV_CPUTIME(init_task)
#ifdef CONFIG_VIRT_CPU_ACCOUNTING_GEN
	.vtime.seqcount	= SEQCNT_ZERO(init_task.vtime_seqcount),
	.vtime.starttime = 0,
	.vtime.state	= VTIME_SYS,
#endif
#ifdef CONFIG_NUMA_BALANCING
	.numa_preferred_nid = -1,
	.numa_group	= NULL,
	.numa_faults	= NULL,
#endif
#ifdef CONFIG_KASAN
	.kasan_depth	= 1,
#endif
#ifdef CONFIG_TRACE_IRQFLAGS
	.softirqs_enabled = 1,
#endif
#ifdef CONFIG_LOCKDEP
	.lockdep_recursion = 0,
#endif
#ifdef CONFIG_FUNCTION_GRAPH_TRACER
	.ret_stack	= NULL,
#endif
#if defined(CONFIG_TRACING) && defined(CONFIG_PREEMPT)
	.trace_recursion = 0,
#endif
#ifdef CONFIG_LIVEPATCH
	.patch_state	= KLP_UNDEFINED,
#endif
#ifdef CONFIG_SECURITY
	.security	= NULL,
#endif
#else /* CONFIG_SCHED_PDS */
#ifdef CONFIG_THREAD_INFO_IN_TASK
	.thread_info	= INIT_THREAD_INFO(init_task),
	.stack_refcount	= ATOMIC_INIT(1),
#endif
	.state		= 0,
	.stack		= init_stack,
	.usage		= ATOMIC_INIT(2),
	.flags		= PF_KTHREAD,
	.prio		= MAX_PRIO - 20,
	.static_prio	= MAX_PRIO - 20,
	.normal_prio	= MAX_PRIO - 20,
	.policy		= SCHED_NORMAL,
	.cpus_allowed	= CPU_MASK_ALL,
	.nr_cpus_allowed= NR_CPUS,
	.mm		= NULL,
	.active_mm	= &init_mm,
	.restart_block	= {
		.fn = do_no_restart_syscall,
	},
	.se		= {
		.group_node 	= LIST_HEAD_INIT(init_task.se.group_node),
	},
	.rt		= {
		.run_list	= LIST_HEAD_INIT(init_task.rt.run_list),
		.time_slice	= RR_TIMESLICE,
	},
	.tasks		= LIST_HEAD_INIT(init_task.tasks),
#ifdef CONFIG_SMP
	.pushable_tasks	= PLIST_NODE_INIT(init_task.pushable_tasks, MAX_PRIO),
#endif
#ifdef CONFIG_CGROUP_SCHED
	.sched_task_group = &root_task_group,
#endif
	.ptraced	= LIST_HEAD_INIT(init_task.ptraced),
	.ptrace_entry	= LIST_HEAD_INIT(init_task.ptrace_entry),
	.real_parent	= &init_task,
	.parent		= &init_task,
	.children	= LIST_HEAD_INIT(init_task.children),
	.sibling	= LIST_HEAD_INIT(init_task.sibling),
	.group_leader	= &init_task,
	RCU_POINTER_INITIALIZER(real_cred, &init_cred),
	RCU_POINTER_INITIALIZER(cred, &init_cred),
	.comm		= INIT_TASK_COMM,
	.thread		= INIT_THREAD,
	.fs		= &init_fs,
	.files		= &init_files,
	.signal		= &init_signals,
	.sighand	= &init_sighand,
	.nsproxy	= &init_nsproxy,
	.pending	= {
		.list = LIST_HEAD_INIT(init_task.pending.list),
		.signal = {{0}}
	},
	.blocked	= {{0}},
	.alloc_lock	= __SPIN_LOCK_UNLOCKED(init_task.alloc_lock),
	.journal_info	= NULL,
	INIT_CPU_TIMERS(init_task)
	.pi_lock	= __RAW_SPIN_LOCK_UNLOCKED(init_task.pi_lock),
	.timer_slack_ns = 50000, /* 50 usec default slack */
	.pids = {
		[PIDTYPE_PID]  = INIT_PID_LINK(PIDTYPE_PID),
		[PIDTYPE_PGID] = INIT_PID_LINK(PIDTYPE_PGID),
		[PIDTYPE_SID]  = INIT_PID_LINK(PIDTYPE_SID),
	},
	.thread_group	= LIST_HEAD_INIT(init_task.thread_group),
	.thread_node	= LIST_HEAD_INIT(init_signals.thread_head),
#ifdef CONFIG_AUDITSYSCALL
	.loginuid	= INVALID_UID,
	.sessionid	= (unsigned int)-1,
#endif
#ifdef CONFIG_PERF_EVENTS
	.perf_event_mutex = __MUTEX_INITIALIZER(init_task.perf_event_mutex),
	.perf_event_list = LIST_HEAD_INIT(init_task.perf_event_list),
#endif
#ifdef CONFIG_PREEMPT_RCU
	.rcu_read_lock_nesting = 0,
	.rcu_read_unlock_special.s = 0,
	.rcu_node_entry = LIST_HEAD_INIT(init_task.rcu_node_entry),
	.rcu_blocked_node = NULL,
#endif
#ifdef CONFIG_TASKS_RCU
	.rcu_tasks_holdout = false,
	.rcu_tasks_holdout_list = LIST_HEAD_INIT(init_task.rcu_tasks_holdout_list),
	.rcu_tasks_idle_cpu = -1,
#endif
#ifdef CONFIG_CPUSETS
	.mems_allowed_seq = SEQCNT_ZERO(init_task.mems_allowed_seq),
#endif
#ifdef CONFIG_RT_MUTEXES
	.pi_waiters	= RB_ROOT_CACHED,
	.pi_top_task	= NULL,
#endif
	INIT_PREV_CPUTIME(init_task)
#ifdef CONFIG_VIRT_CPU_ACCOUNTING_GEN
	.vtime.seqcount	= SEQCNT_ZERO(init_task.vtime_seqcount),
	.vtime.starttime = 0,
	.vtime.state	= VTIME_SYS,
#endif
#ifdef CONFIG_NUMA_BALANCING
	.numa_preferred_nid = -1,
	.numa_group	= NULL,
	.numa_faults	= NULL,
#endif
#ifdef CONFIG_KASAN
	.kasan_depth	= 1,
#endif
#ifdef CONFIG_TRACE_IRQFLAGS
	.softirqs_enabled = 1,
#endif
#ifdef CONFIG_LOCKDEP
	.lockdep_recursion = 0,
#endif
#ifdef CONFIG_FUNCTION_GRAPH_TRACER
	.ret_stack	= NULL,
#endif
#if defined(CONFIG_TRACING) && defined(CONFIG_PREEMPT)
	.trace_recursion = 0,
#endif
#ifdef CONFIG_LIVEPATCH
	.patch_state	= KLP_UNDEFINED,
#endif
#ifdef CONFIG_SECURITY
	.security	= NULL,
#endif
#endif /* CONFIG_SCHED_PDS */
};
EXPORT_SYMBOL(init_task);

/*
 * Initial thread structure. Alignment of this is handled by a special
 * linker map entry.
 */
union thread_union init_thread_union __init_task_data =
	{ INIT_THREAD_INFO(init_task) };
