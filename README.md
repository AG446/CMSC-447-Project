# CMSC-447-Project
Where to store our code updates throughout CMSC 447 project lifecycle

# Coding Standards

## 📌 General Coding Practices
- Code should be **clear, concise, and maintainable**.
- Follow a **consistent style** across all projects.
- Document **important logic** and non-trivial functions with comments.
- Write **self-explanatory code**—avoid unnecessary comments for obvious statements.

---

## 🔤 Naming Conventions
- **Use snake_case** for variables, functions, and file names.
  - ✅ `user_id`, `fetch_data_from_api()`
  - ❌ `userId`, `fetchDataFromAPI()`
- **Use PascalCase** for class names.
  - ✅ `DataProcessor`
  - ❌ `data_processor`
- **Use UPPER_CASE** for constants.
  - ✅ `MAX_RETRIES = 5`
  - ❌ `maxRetries = 5`
- **Variable names should be descriptive** and indicate purpose.
  - ✅ `total_users`, `error_message`
  - ❌ `x`, `data`, `temp`

---

## 📏 Formatting & Structure
- Use **4 spaces per indentation level** (no tabs).
- Keep **line length under 100 characters**.
- Write **meaningful commit messages**:
  - ✅ `fix: resolve login timeout issue`
  - ❌ `fixed bug`
  
---

## 🌱 Branching & Version Control
- Develop each feature or fix in a **feature branch** before merging.
- Use **descriptive branch names**:
  - ✅ `sprint/current-task`
  - ❌ `new-feature`
- Before merging, ensure:
  - ✅ Code is reviewed.
  - ✅ Automated tests pass.
  - ✅ No unnecessary console logs or debug code remain.

---

## 📜 Code Documentation
- **Use docstrings/comments** for public functions and classes.
- Follow this format:
  ```python
  def calculate_total_price(items: list) -> float:
      """
      Calculate the total price of a list of items.
      
      Args:
          items (list): A list of item prices.

      Returns:
          float: The total price.
      """
