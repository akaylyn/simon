/*
 *  Manage a string by using a generic, dynamic queue data structure.
 *
 *  Copyright (C) 2010  Efstathios Chatzikyriakidis (contact@efxa.org)
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

// include queue library header.
#include <QueueArray.h>
#include <Streaming.h>

typedef struct {
  byte flame; 
  byte effect; 
} ts;

typedef struct {
  byte red;
  byte green;
  byte blue;
} ci;

typedef struct {
  void * ptr;
  int len;
} resend;

void setup ();
void loop ();
void add(resend &r, ts &m);
void add(resend &r, ci &m);

// create a queue 
QueueArray <resend> que;

// startup point entry (runs once).
void setup () {
  // start serial communication.
  Serial.begin (115200);

  ts ts1, ts2, ts3;
  ci ci1, ci2, ci3;
  resend rs[6];
  
  ts1.flame = 100;
  
  add(rs[0], ts1);
  add(rs[1], ts2);
  add(rs[2], ts3);
  add(rs[3], ci1);
  add(rs[4], ci2);
  add(rs[5], ci3);
  
  // enqueue all the message's characters to the queue.
  for (int i = 0; i < 6; i++)
    que.push(rs[i]);

  // dequeue all the message's characters from the queue.
  while (!que.isEmpty ()) {
    resend r = que.pop();
    Serial << "len: " << r.len << " contains: ";
    for( byte i=0;i<r.len;i++) {
      byte *p = static_cast<byte *>(r.ptr) + i;
      byte foo;
      memcpy(&foo, p, sizeof(foo));
      Serial << foo << " ";
    }
    Serial << endl;
  }

}

// loop the main sketch.
void loop () {
  // nothing here.
}

void add(resend &r, ts &m) {
  r.ptr = &m;
  r.len = sizeof(m); 
}
void add(resend &r, ci &m) {
  r.ptr = &m;
  r.len = sizeof(m); 
}
