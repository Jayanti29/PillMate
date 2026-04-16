![Status](https://img.shields.io/badge/Project-Work%20In%20Progress-ffae42?style=for-the-badge&logo=github)
# PillMate
A full-stack healthcare app that helps users manage medicines, track doses, get reminders, and monitor stock — powered by Data Structures &amp; Algorithms.
# 💊 PillMate – Smart Medicine Reminder & Pill Tracker

PillMate is a full-stack healthcare application designed to help users **manage medicines, track daily doses, receive reminders, and monitor stock levels**.

It combines a **modern React frontend** with a **C backend powered by Data Structures & Algorithms (DSA)** to demonstrate real-world problem solving.

---

## 🚀 Features

### 🏠 Dashboard
- Daily greeting & overview
- Medicine statistics
- Low stock alerts
- Quick actions

### 💊 Medicine Management
- Add, delete, and view medicines
- Track dosage, frequency, and timings
- Pill progress bars

### ⏰ Smart Reminder System
- Time-based reminders
- Missed dose alerts
- Real-time notifications

### 🚨 Refill Alerts
- Alerts when medicine stock is low
- Priority-based urgency system

### 📅 Timeline View
- Morning / Afternoon / Night schedule
- Shows taken, missed, and pending doses

### 🤖 AI Assistant (Coming Soon)
- Chat-based health assistant
- (⚠️ API not integrated yet — currently non-functional)

---

## ⚠️ Important Note

> The AI feature is currently **not working** because the API has not been integrated yet.  
> The UI and structure are ready — backend/API connection will be added in future updates.

---

## 🧠 Data Structures Used (Backend in C)

| Data Structure | Purpose |
|---------------|--------|
| Linked List | Stores medicines dynamically |
| Queue | Manages reminder scheduling (FIFO) |
| Priority Queue | Handles urgent refill alerts |
| Hash Table | Enables fast medicine search (O(1)) |

---

## 🛠️ Tech Stack

### Frontend
- React (CDN)
- HTML, CSS
- Babel (in-browser JSX)

### Backend
- C Programming
- DSA (Linked List, Queue, PQ, Hash Table)

### Storage
- JSON (`medicines.json`)

---

## 📂 Project Structure
pillmate/
├── frontend/
│ └── index.html
├── backend/
│ └── pillmate.c
├── medicines.json
├── run.sh
└── README.md

