# 🍰 Pastry Shop Order Management Simulator

## 📖 Project Overview
This project has been developed as the final project of an university course of "Algorithms and Data Structure 2023-2024" at Politecnico di Milano.
This project simulates the operations of an **industrial pastry shop** aiming to improve its **order management system**.  
The simulation runs in **discrete time**, starting from time `0`. Each executed command advances the time by one unit.

The system manages:
- 📦 **Ingredients** stored in a warehouse, each with an expiration date.  
- 📘 **Recipes**, each requiring specific quantities of ingredients.  
- 🧾 **Orders** placed by customers via phone or online.  
- 🚚 **Courier pickups**, periodic with limited truck capacity.  

The simulation ends after the last command is processed.

---

## ⚙️ Features

### Ingredients & Recipes
- Recipes have a **unique name** and a list of required ingredients with quantities (in grams).  
- Ingredient lots have a **quantity** and an **expiration time**.  

### Orders
- Orders are **accepted instantly** if enough ingredients are available.  
- If not, orders go into a **waiting queue** (FIFO).  
- Orders are prepared as soon as stock allows.  

### Courier Deliveries
- A courier arrives every **N time units**.  
- The truck has a **maximum capacity (grams)**.  
- Orders are selected in **chronological order** until the next order does not fit.  
- Selected orders are loaded in **descending weight order**; ties are broken chronologically.  
- If no orders are ready, the program outputs: 'camioncino vuoto'

---

## 🛠️ Implementation Notes
- All quantities are positive integers.  
- Names (ingredients, recipes) can include `{a–z, A–Z, 0–9, _}` up to **255 characters**.  
- Integer values fit within **32 bits**.  
- Expired lots are automatically discarded.  



