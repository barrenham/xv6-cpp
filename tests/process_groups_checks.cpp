module;
#include <atomic>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <vector>

module xv.process;
import :groups;
import :group_platform;

namespace xv::process::group_test
{
std::atomic<int> pages{0};
std::atomic<int> budget{-1};

void check(bool ok, const char *message)
{
  if (!ok)
  {
    std::fprintf(stderr, "FAIL: %s\n", message);
    std::exit(1);
  }
}

void rules_and_lifetime()
{
  class Count final : public GroupVisitor
  {
  public:

    void visit(ProcessMembership &) override
    {
      ++members;
    }

    unsigned members = 0;
  };

  ProcessGroups groups;
  groups.initialize();
  ProcessMembership root, child, sibling, grandchild;
  check(groups.create_initial(root, 1), "initial session");
  check(groups.get_group(root) == 1 && groups.get_session(root, 0) == 1,
        "initial group and session");
  check(groups.create_session(root) == -1 && groups.set_group(root, 0, 0) == -1,
        "session leader restrictions");
  groups.inherit(child, 2, root);
  groups.inherit(sibling, 3, root);
  check(groups.get_group(child) == 1 && groups.get_session(root, 2) == 1,
        "fork inheritance and query by PID");
  check(groups.set_group(root, -1, 0) == -1 && groups.set_group(root, 2, -1) == -1 &&
            groups.set_group(root, 2, 99) == -1 && groups.get_session(root, -1) == -1 &&
            groups.get_session(root, 99) == -1,
        "invalid IDs and nonexistent groups");
  check(groups.set_group(root, 2, 0) == 0 && groups.get_group(child) == 2,
        "parent creates child's group");
  check(groups.set_group(root, 3, 2) == 0 && groups.get_group(sibling) == 2,
        "join same-session group");
  Count selected, current, missing;
  groups.visit_group(root, 2, selected);
  groups.visit_group(child, 0, current);
  groups.visit_group(root, 99, missing);
  check(selected.members == 2 && current.members == 2 && missing.members == 0,
        "explicit/current group selection visits exactly its members");
  groups.inherit(grandchild, 4, child);
  check(groups.set_group(root, 4, 1) == -1, "only direct children");
  // Reaping the group leader cannot retire its group while members remain.
  groups.release(child);
  check(groups.get_group(grandchild) == 2 && groups.get_session(root, 4) == 1,
        "group survives leader reaping");
  groups.reparent(grandchild, 1);
  check(groups.set_group(root, 4, 1) == 0, "adopted child authorization");
  groups.release(sibling);
  check(groups.set_group(root, 4, 2) == -1, "empty group disappears");
  check(groups.create_session(grandchild) == 4, "new session and group");
  check(groups.get_group(grandchild) == 4 && groups.get_session(root, 4) == 4,
        "cross-session getsid is permitted");
  check(groups.set_group(root, 4, 1) == -1 && groups.set_group(grandchild, 0, 1) == -1,
        "session boundaries and leader immobility");
  ProcessMembership survivor;
  groups.inherit(survivor, 5, grandchild);
  groups.release(grandchild);
  check(groups.get_session(survivor, 0) == 4 && groups.get_group(survivor) == 4,
        "session survives leader reaping");
  check(groups.set_group(survivor, 0, 0) == 0 && groups.get_group(survivor) == 5,
        "last member moves to a new group in the same session");
  groups.release(survivor);
  groups.release(root);
  check(pages == 0, "all group and session pages reclaimed");
}

void exec_and_allocation()
{
  ProcessGroups groups;
  groups.initialize();
  ProcessMembership root, child, grandchild;
  budget = 0;
  check(!groups.create_initial(root, 1) && pages == 0, "initial allocation failure");
  budget = 1;
  check(!groups.create_initial(root, 1) && pages == 0, "initial allocation rollback");
  budget = -1;
  check(groups.create_initial(root, 1), "initial allocation retry");
  groups.inherit(child, 2, root);
  budget = 0;
  check(groups.set_group(root, 2, 0) == -1 && groups.get_group(child) == 1,
        "failed group allocation preserves identity");
  check(groups.set_group(root, 2, 1) == 0, "existing group needs no allocation");
  check(groups.create_session(child) == -1 && pages == 2,
        "session OOM preserves identity");
  budget = 1;
  check(groups.create_session(child) == -1 && pages == 2 &&
            groups.get_session(child, 0) == 1,
        "session group failure rolls back");
  budget = -1;
  groups.did_exec(child);
  check(groups.get_group(child) == 1 && groups.get_session(child, 0) == 1,
        "exec preserves group and session");
  check(groups.set_group(root, 2, 1) == -1 && groups.set_group(child, 0, 0) == 0,
        "exec bars parent changes but permits self changes");
  groups.inherit(grandchild, 3, child);
  check(groups.set_group(child, 3, 0) == 0, "fork resets child exec marker");
  // A former leader still cannot setsid while its old group has members.
  check(groups.set_group(grandchild, 0, 2) == 0 && groups.set_group(child, 0, 1) == 0 &&
            groups.create_session(child) == -1,
        "setsid rejects a surviving group named after the caller");
  groups.release(grandchild);
  check(groups.create_session(child) == 2, "setsid after old group becomes empty");
  groups.release(child);
  groups.release(root);
  check(pages == 0, "allocation and exec tests reclaim all pages");
}

void many_groups()
{
  ProcessGroups groups;
  groups.initialize();
  ProcessMembership root;
  std::array<ProcessMembership, 192> members;
  check(groups.create_initial(root, 1), "many groups initial session");
  for (unsigned i = 0; i < members.size(); ++i)
  {
    groups.inherit(members[i], 100 + i, root);
    check(groups.set_group(members[i], 0, 0) == 0, "more than 64 simultaneous groups");
  }
  check(pages == 194, "one session shared by 193 groups");
  groups.release(root);
  for (auto &member : members)
  {
    check(groups.get_session(member, 0) == 1,
          "session survives original group removal");
    groups.release(member);
  }
  check(pages == 0, "last group releases shared session");
}

void terminals()
{
  ProcessGroups groups;
  groups.initialize();
  ProcessMembership root, child, other, survivor;
  int terminal, second_terminal;
  check(groups.create_initial(root, 1), "terminal initial session");
  groups.inherit(child, 2, root);
  groups.inherit(other, 3, root);
  check(groups.foreground(root, &terminal) == -1, "no implicit controlling terminal");
  check(groups.claim_terminal(child, &terminal) == -1, "only session leader claims");
  check(groups.claim_terminal(root, &terminal) == 0 &&
            groups.claim_terminal(root, &terminal) == 0 &&
            groups.claim_terminal(root, &second_terminal) == -1,
        "exclusive session binding");
  check(groups.set_group(root, 2, 0) == 0 &&
            groups.set_foreground(root, &terminal, 2) == 0,
        "handoff foreground group");
  check(!groups.may_read_terminal(root, &terminal) &&
            groups.may_read_terminal(child, &terminal),
        "background reads rejected");
  check(groups.create_session(other) == 3 &&
            groups.claim_terminal(other, &terminal) == -1 &&
            groups.set_foreground(other, &terminal, 3) == -1 &&
            groups.set_foreground(root, &terminal, 3) == -1,
        "cross-session isolation");
  groups.release(child);
  check(groups.foreground(root, &terminal) == 0 &&
            groups.set_foreground(root, &terminal, 1) == 0,
        "empty foreground clears and reclaims");
  groups.exit_terminal(root);
  check(groups.foreground(root, &terminal) == -1 &&
            groups.claim_terminal(other, &terminal) == 0,
        "leader exit releases terminal");
  groups.inherit(survivor, 4, other);
  groups.exit_terminal(other);
  check(groups.foreground(survivor, &terminal) == -1 &&
            groups.claim_terminal(root, &terminal) == 0,
        "surviving session does not pin terminal");
  groups.release(other);
  groups.release(survivor);
  groups.release(root);
  check(pages == 0, "terminal ownership leaves no session pages");
}

void concurrent()
{
  ProcessGroups groups;
  groups.initialize();
  ProcessMembership root;
  check(groups.create_initial(root, 1), "concurrent initial session");
  std::atomic<bool> stop{false};
  std::thread observer(
      [&]
      {
        while (!stop.load())
        {
          for (int id = 100; id < 104; ++id)
          {
            int session = groups.get_session(root, id);
            check(session == -1 || session == 1, "concurrent PID queries");
          }
        }
      });
  std::vector<std::thread> workers;
  for (int index = 0; index < 4; ++index)
  {
    workers.emplace_back(
        [&, index]
        {
          for (int round = 0; round < 200; ++round)
          {
            ProcessMembership member;
            groups.inherit(member, 100 + index, root);
            check(groups.set_group(member, 0, 0) == 0, "concurrent group creation");
            groups.did_exec(member);
            check(groups.set_group(root, 100 + index, 1) == -1,
                  "parent cannot move an execed child");
            check(groups.set_group(member, 0, 1) == 0, "concurrent group removal");
            groups.release(member);
          }
        });
  }
  for (auto &worker : workers)
  {
    worker.join();
  }
  stop = true;
  observer.join();
  for (int round = 0; round < 50; ++round)
  {
    ProcessMembership child;
    groups.inherit(child, 1000, root);
    int result;
    std::thread exec([&] { groups.did_exec(child); });
    std::thread set([&] { result = groups.set_group(root, 1000, 0); });
    exec.join();
    set.join();
    check((result == 0 || result == -1) && groups.set_group(root, 1000, 1) == -1,
          "exec/setpgid linearization");
    groups.release(child);
  }
  groups.release(root);
  check(pages == 0, "concurrent operations reclaim all pages");
}
} // namespace xv::process::group_test

namespace xv::process::detail
{
void *allocate_group_object(unsigned long bytes, unsigned long alignment)
{
  if (group_test::budget == 0)
  {
    return nullptr;
  }
  if (group_test::budget > 0)
  {
    --group_test::budget;
  }
  (void)bytes;
  (void)alignment;
  auto *page = std::aligned_alloc(4096, 4096);
  if (page)
  {
    ++group_test::pages;
  }
  return page;
}

void free_group_object(void *page)
{
  std::free(page);
  --group_test::pages;
}

[[noreturn]] void group_fail(const char *message)
{
  std::fprintf(stderr, "group panic: %s\n", message);
  std::abort();
}
} // namespace xv::process::detail

extern "C" int run_process_group_contracts()
{
  xv::process::group_test::rules_and_lifetime();
  xv::process::group_test::exec_and_allocation();
  xv::process::group_test::many_groups();
  xv::process::group_test::terminals();
  xv::process::group_test::concurrent();
  std::puts("process groups and sessions: passed");
  return 0;
}
