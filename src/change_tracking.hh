/* -*-	Mode:C++ -*- */

/*
 * change_tracking.hh
 * Copyright (C) 1999 by John Heidemann
 * $Id: change_tracking.hh,v 1.15 2004/12/10 20:36:18 johnh Exp $
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 *
 */


#ifndef lavaps_change_tracking_h
#define lavaps_change_tracking_h

/*
 * track a variable, remembering it's last value
 */
template <class T>
class change_tracking {
protected:
	T last_;
	T change_;
public:
	// gcc won't accept :last_(0)
	// with error ``only constructors take base initializers''
	change_tracking() { change_ = last_ = (T)0; }   // xxx: should track whether we're init'ed
	change_tracking(T first) { change_ = last_ = first; }   // the last should be first
	void tick_set(T current) { change_ = T(current - last_); last_ = current; }
	T operator=(T current) { tick_set(current); return current; } // assignment
	void tick_incr(T delta) { change_ = delta; last_ += delta; }
	void commit() { change_ = (T)0; }
	T current() { return last_; }
	T change() { return change_; }
	bool changed() { return change_ != 0; }
};

/*
 * track a variable, remembering the value before it was explicitly reset
 * (via clear_change())
 */
template <class T>
class explicit_change_tracking {
protected:
	T last_;
	T current_;
public:
	explicit_change_tracking() { current_ = last_ = (T)0; }
	explicit_change_tracking(T first) { current_ = last_ = first; }
	void tick_set(T current) { current_ = current; }
	T* address() { return &current_; }  // allow users to directly change current_
	// T operator=(T current) { tick_set(current); return current; } // assignment
	void tick_incr(T delta) { current_ += delta; }  // DOES NOT COMMIT (incompatiblility with simple change_tracking<>)
	void tick_scale(T delta) { current_ *= delta; }
	void commit() { last_ = current_; }  // commit
	T current() { return current_; }
	T change() { return current_ - last_; }
	bool changed() { return current_ != last_; }
};

/*
 * like change tracking, but also keeps a running sum of changes
 */
template <class T, class SUM>
class sum_change_tracking : public change_tracking<T> {
protected:
	static SUM s_;
	void update_sum() { s_.sum_ += this->change_; }
public:
	sum_change_tracking() {};
	sum_change_tracking(T first) : change_tracking<T>(first) { update_sum(first); };
	~sum_change_tracking() { tick_incr(-change_tracking<T>::last_); };


	void tick_set(T current) { change_tracking<T>::tick_set(current); update_sum(); };
	void tick_incr(T delta) { change_tracking<T>::tick_incr(delta); update_sum(); };

	T sum() { return s_.sum_; }
};

#endif /* lavaps_change_tracking_h */
