/*  Copyright (C) 2011-2014 Dmitry Shatrov - All Rights Reserved
    e-mail: info@momentvideo.org

    Unauthorized copying of this file or any part of its contents, 
    via any medium is strictly prohibited.

    Proprietary and confidential.
 */


#ifndef LIBMARY__SELECT_POLL_GROUP__H__
#define LIBMARY__SELECT_POLL_GROUP__H__


#include <libmary/types.h>
#include <libmary/libmary_thread_local.h>
#include <libmary/cb.h>
#include <libmary/intrusive_list.h>
#include <libmary/active_poll_group.h>
#include <libmary/state_mutex.h>


namespace M {

// SelectPollGroup is useless on Win32 because select() can only wait for sockets
// on Windows, hence there's no way to implement ActivePollGroup::trigger()
// in an acceptable way.

class SelectPollGroup : public ActivePollGroup
{
private:
    StateMutex mutex;

    class PollableList_name;
    class SelectedList_name;

    // TODO It would be more efficient to manage PollableEntries through
    // a deletion queue instead of basic refcounts. There's two points where
    // the queue can be processed: right after processing all events in poll(),
    // and in ~SelectPollGroup() destructor. That way there would be no need
    // in unref loops when leaving poll().
    //
    // Operations with StReferenced base (ref/unref) should be synchronized
    // with SelectPollGroup::mutex.
    class PollableEntry : public StReferenced,
			  public IntrusiveListElement<PollableList_name>,
			  public IntrusiveListElement<SelectedList_name>
    {
    public:
	mt_const SelectPollGroup *select_poll_group;

	mt_const Cb<Pollable> pollable;
	mt_const int fd;

	mt_mutex (mutex) bool valid;

	mt_mutex (mutex) bool need_input;
	mt_mutex (mutex) bool need_output;
    };

    typedef IntrusiveList< PollableEntry, PollableList_name > PollableList;
    typedef IntrusiveList< PollableEntry, SelectedList_name > SelectedList;

    mt_const LibMary_ThreadLocal *poll_tlocal;

    mt_mutex (mutex) PollableList pollable_list;

    mt_const int trigger_pipe [2];
    mt_mutex (mutex) bool triggered;
    mt_mutex (mutex) bool block_trigger_pipe;

    mt_sync_domain (poll) bool got_deferred_tasks;

  mt_iface (PollGroup::Feedback)
    static Feedback const pollable_feedback;
    static void requestInput  (void *_pollable_entry);
    static void requestOutput (void *_pollable_entry);
  mt_iface_end

    mt_unlocks (mutex) mt_throws Result doTrigger ();

public:
  mt_iface (ActivePollGroup)
    mt_iface (PollGroup)
      mt_throws PollableKey addPollable (CbDesc<Pollable> const &pollable,
                                         bool auto_remove = false);

      mt_throws Result addPollable_beforeConnect (CbDesc<Pollable> const & /* pollable */,
                                                  PollableKey * const /* ret_key */,
                                                  bool          const /* auto_remove */ = false)
          { return Result::Success; }

      mt_throws Result addPollable_afterConnect (CbDesc<Pollable> const &pollable,
                                                 PollableKey * const ret_key,
                                                 bool          const /* auto_remove */ = false)
      {
          PollableKey const key = addPollable (pollable);
          if (!key)
              return Result::Failure;

          if (ret_key)
              *ret_key = key;

          return Result::Success;
      }

      void removePollable (PollableKey mt_nonnull key);
    mt_iface_end

    // Must be called from the same thread every time.
    mt_throws Result poll (Uint64 timeout_microsec = (Uint64) -1);

    mt_throws Result trigger ();
  mt_iface_end

    mt_throws Result open ();

    mt_const void bindToThread (LibMary_ThreadLocal * const poll_tlocal)
        { this->poll_tlocal = poll_tlocal; }

     SelectPollGroup (EmbedContainer *embed_container);
    ~SelectPollGroup ();
};

}


#endif /* LIBMARY__SELECT_POLL_GROUP__H__ */

