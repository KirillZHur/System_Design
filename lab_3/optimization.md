# Оптимизация SQL-запросов

В данном разделе рассмотрены основные SQL-запросы системы и показано,
как использование индексов влияет на план выполнения запросов.

## 1. Поиск пользователя по логину

### Запрос
```sql
SELECT id, login, first_name, last_name, created_at
FROM users
WHERE login = 'ivan.petrov';
```

### До оптимизации

Без уникального индекса по полю login PostgreSQL выполняет последовательное сканирование таблицы (Seq Scan), так как для поиска требуется просмотр всех записей.

План выполнения ДО:
```
"Seq Scan on users  (cost=0.00..10.75 rows=1 width=670)"
"  Filter: ((login)::text = 'ivan.petrov'::text)"
```

### После оптимизации

После добавления ограничения UNIQUE на поле login PostgreSQL автоматически создаёт уникальный индекс, который используется для быстрого поиска.

План выполнения ПОСЛЕ:
```
"Index Scan using users_login_key on users  (cost=0.14..8.15 rows=1 width=670)"
"  Index Cond: ((login)::text = 'ivan.petrov'::text)"
```

Вывод:

Поиск пользователя по логину выполняется быстрее за счёт использования индекса. Это особенно важно для операций аутентификации и поиска пользователя.

## 2. Поиск пользователя по имени и фамилии

### Запрос
```sql
SELECT id, login, first_name, last_name, created_at
FROM users
WHERE last_name = 'Петров'
  AND first_name = 'Иван';
```

### До оптимизации

Без составного индекса PostgreSQL выполняет последовательное сканирование таблицы (Seq Scan), так как для поиска требуется просмотр всех записей.

План выполнения ДО:
```
"Seq Scan on users  (cost=0.00..1.15 rows=1 width=670)"
"  Filter: (((last_name)::text = 'Петров'::text) AND ((first_name)::text = 'Иван'::text))"
```
### После оптимизации

После создания составного индекса:
```sql 
CREATE INDEX idx_users_last_name_first_name
ON users(last_name, first_name);
```
PostgreSQL может использовать индекс для ускорения поиска.

План выполнения ПОСЛЕ:
```
"Index Scan using idx_users_last_name_first_name on users  (cost=0.14..8.15 rows=1 width=670)"
"  Index Cond: (((last_name)::text = 'Петров'::text) AND ((first_name)::text = 'Иван'::text))"
```

Вывод

Составной индекс позволяет ускорить поиск пользователя по фамилии и имени. При увеличении объёма данных PostgreSQL будет автоматически использовать индекс, что существенно повышает производительность запроса.


## 3. Поиск курсов по автору

### Запрос
```sql
SELECT id, title, description, teacher_id, created_at
FROM courses
WHERE teacher_id = 1;
```

### До оптимизации

Без индекса по полю teacher_id PostgreSQL выполняет последовательное сканирование таблицы (Seq Scan), так как для поиска требуется просмотр всех записей.

План выполнения ДО:
```
"Seq Scan on courses  (cost=0.00..12.00 rows=1 width=474)"
"  Filter: (teacher_id = 1)"
```

### После оптимизации

После создания индекса:
```sql
CREATE INDEX idx_courses_teacher_id
    ON courses(teacher_id);
```

PostgreSQL может использовать индекс для быстрого поиска курсов по автору.

План выполнения ПОСЛЕ:
```
"Index Scan using idx_courses_teacher_id on courses  (cost=0.14..8.15 rows=1 width=474)"
"  Index Cond: (teacher_id = 1)"
```
Вывод

Индекс по полю teacher_id ускоряет поиск курсов, созданных конкретным пользователем.

## 4. Получение записей на курс

### Запрос
```sql
SELECT id, user_id, course_id, enrolled_at
FROM course_enrollments
WHERE course_id = 1;
```

### До оптимизации

Без индекса по полю course_id PostgreSQL выполняет последовательное сканирование таблицы.

План выполнения ДО:
```
"Seq Scan on course_enrollments  (cost=0.00..27.00 rows=7 width=32)"
"  Filter: (course_id = 1)"
```

### После оптимизации

После создания индекса:
```sql
CREATE INDEX idx_course_enrollments_course_id
    ON course_enrollments(course_id);
```
План выполнения ПОСЛЕ:
```
"Index Scan using idx_course_enrollments_course_id on course_enrollments  (cost=0.14..8.15 rows=1 width=32)"
"  Index Cond: (course_id = 1)"
```
Вывод

Индекс по полю course_id ускоряет поиск пользователей, записанных на конкретный курс.

## 5. Получение прогресса по уроку

### Запрос
```sql
SELECT id, user_id, lesson_id, is_completed, completed_at
FROM lesson_progress
WHERE lesson_id = 1;
```

### До оптимизации

Без индекса по полю lesson_id PostgreSQL выполняет последовательное сканирование таблицы.

План выполнения ДО:
```
"Seq Scan on lesson_progress  (cost=0.00..26.62 rows=7 width=33)"
"  Filter: (lesson_id = 1)"
```

### После оптимизации

После создания индекса:
```sql
CREATE INDEX idx_lesson_progress_lesson_id
    ON lesson_progress(lesson_id);
```
План выполнения ПОСЛЕ:
```
"Index Scan using idx_lesson_progress_lesson_id on lesson_progress  (cost=0.14..8.15 rows=1 width=33)"
"  Index Cond: (lesson_id = 1)"
```
Вывод

Индекс по полю lesson_id ускоряет получение информации о прохождении конкретного урока.

## Общий вывод

В ходе работы были рассмотрены основные запросы, для которых индексы
создавались вручную.

Остальные операции системы имеют схожий характер и используют поля,
для которых в базе данных уже существуют индексы, создаваемые автоматически
ограничениями `PRIMARY KEY` и `UNIQUE`.

В частности:
- получение уроков курса использует уникальный индекс `(course_id, position)`;
- операции с таблицами `course_enrollments` и `lesson_progress`
используют составные уникальные индексы.

Таким образом, для всех ключевых операций базы данных обеспечено
использование индексов, что позволяет избежать полного сканирования таблиц
и повысить производительность системы при увеличении объёма данных.