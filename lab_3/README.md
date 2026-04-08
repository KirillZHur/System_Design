# Задание №3 по предмету System Design

## Выполнил: Журавлев Кирилл М8О-102СВ-25

## Вариант 19: Система управления обучением

## 1. Проектирование схемы базы данных

### Сущности и таблицы

Для реализации варианта используются следующие таблицы:

- `users` — пользователи системы;
- `courses` — курсы;
- `lessons` — уроки, принадлежащие курсу;
- `course_enrollments` — запись пользователей на курсы;
- `lesson_progress` — информация о прохождении уроков пользователями.

Дополнительные таблицы `course_enrollments` и `lesson_progress` введены для хранения связей многие-ко-многим между пользователями и курсами, а также между пользователями и уроками.

#### Таблица `users`

Хранит данные пользователей системы.

Поля:
- `id` BIGSERIAL — первичный ключ;
- `login` VARCHAR(100) — уникальный логин пользователя;
- `password_hash` VARCHAR(255) — хеш пароля;
- `first_name` VARCHAR(100) — имя;
- `last_name` VARCHAR(100) — фамилия;
- `role` TEXT -  роль (student/teacher/admin);
- `created_at` TIMESTAMP — дата и время создания записи.

#### Таблица `courses`

Хранит курсы, доступные в системе.

Поля:
- `id` BIGSERIAL — первичный ключ;
- `title` VARCHAR(200) — название курса;
- `description` TEXT — описание курса;
- `teacher_id` BIGINT — пользователь, создавший курс;
- `created_at` TIMESTAMP — дата и время создания курса.

#### Таблица `lessons`

Хранит уроки, входящие в состав курсов.

Поля:
- `id` BIGSERIAL — первичный ключ;
- `course_id` BIGINT — ссылка на курс;
- `title` VARCHAR(200) — название урока;
- `content` TEXT — содержимое урока;
- `position` INTEGER — порядковый номер урока в курсе;
- `created_at` TIMESTAMP — дата и время создания урока.

#### Таблица `course_enrollments`

Хранит информацию о записи пользователей на курсы.

Поля:
- `id` BIGSERIAL — первичный ключ;
- `user_id` BIGINT — ссылка на пользователя;
- `course_id` BIGINT — ссылка на курс;
- `enrolled_at` TIMESTAMP — дата и время записи на курс.

#### Таблица `lesson_progress`

Хранит информацию о прохождении уроков пользователями.

Поля:
- `id` BIGSERIAL — первичный ключ;
- `user_id` BIGINT — ссылка на пользователя;
- `lesson_id` BIGINT — ссылка на урок;
- `is_completed` BOOLEAN — признак прохождения урока;
- `completed_at` TIMESTAMP — дата и время прохождения урока.

### Первичные ключи

Во всех таблицах используются первичные ключи типа `BIGSERIAL`:

- `users.id`
- `courses.id`
- `lessons.id`
- `course_enrollments.id`
- `lesson_progress.id`

### Внешние ключи

В схеме используются следующие внешние ключи:

- `courses.author_user_id` → `users.id`
- `lessons.course_id` → `courses.id`
- `course_enrollments.user_id` → `users.id`
- `course_enrollments.course_id` → `courses.id`
- `lesson_progress.user_id` → `users.id`
- `lesson_progress.lesson_id` → `lessons.id`

### Выбор типов данных

В проекте используются следующие типы данных:

- `BIGSERIAL` — для первичных ключей;
- `BIGINT` — для внешних ключей;
- `VARCHAR(100)` / `VARCHAR(200)` — для логинов, имён и названий;
- `TEXT` — для описаний курсов и содержимого уроков;
- `TIMESTAMP` — для дат создания, записи и прохождения;
- `BOOLEAN` — для признака прохождения урока.

### Ограничения целостности

В схеме используются следующие ограничения:

#### `users`
- `login` — `NOT NULL`, `UNIQUE`
- `password_hash` — `NOT NULL`
- `first_name` — `NOT NULL`
- `last_name` — `NOT NULL`
- проверка минимальной длины логина: `CHECK (char_length(login) >= 3)`

#### `courses`
- `title` — `NOT NULL`
- `author_user_id` — `NOT NULL`
- проверка минимальной длины названия курса: `CHECK (char_length(title) >= 2)`

#### `lessons`
- `course_id` — `NOT NULL`
- `title` — `NOT NULL`
- `position` — `NOT NULL`
- проверка корректного номера урока: `CHECK (position > 0)`
- уникальность номера урока в пределах курса: `UNIQUE (course_id, position)`

#### `course_enrollments`
- `user_id` — `NOT NULL`
- `course_id` — `NOT NULL`
- пользователь не может быть записан на один и тот же курс дважды: `UNIQUE (user_id, course_id)`

#### `lesson_progress`
- `user_id` — `NOT NULL`
- `lesson_id` — `NOT NULL`
- `is_completed` — `NOT NULL DEFAULT FALSE`
- для одного пользователя и одного урока хранится только одна запись: `UNIQUE (user_id, lesson_id)`

### Связи между таблицами

- `users` → `courses` — связь 1:N, один пользователь может создать несколько курсов;
- `courses` → `lessons` — связь 1:N, один курс может содержать несколько уроков;
- `users` ↔ `courses` — связь M:N через таблицу `course_enrollments`, так как один пользователь может быть записан на несколько курсов, и один курс может иметь несколько пользователей;
- `users` ↔ `lessons` — связь M:N через таблицу `lesson_progress`, так как один пользователь может пройти несколько уроков, и один урок может быть пройден несколькими пользователями.

## 2. Создание базы данных

### Создание PostgreSQL

Для работы с базой данных PostgreSQL был использован Docker, что позволяет
обеспечить изолированную и воспроизводимую среду выполнения.

Для запуска базы данных был создан сервис `postgres` в файле `docker-compose.yml`:

```yaml
services:
  postgres:
    image: postgres:16
    container_name: lms_postgres
    restart: unless-stopped
    environment:
      POSTGRES_DB: lms_db
      POSTGRES_USER: postgres
      POSTGRES_PASSWORD: postgres
    ports:
      - "5432:5432"
    volumes:
      - postgres_data:/var/lib/postgresql/data

volumes:
  postgres_data:
```

### Создание таблиц

Для создания схемы базы данных был подготовлен файл [schema.sql](lms_service/sql/schema.sql).
В нем описаны:

- все таблицы системы;
- первичные ключи;
- внешние ключи;
- ограничения целостности (`NOT NULL`, `UNIQUE`, `CHECK`);
- индексы для ускорения поиска и соединений таблиц.

Порядок создания таблиц выбран с учетом зависимостей между ними:
сначала создается таблица `users`, затем `courses`, `lessons`,
`course_enrollments` и `lesson_progress`.

### Добавление тестовых данных

Для заполнения базы данных подготовлен файл [data.sql](lms_service/sql/data.sql).

Файл содержит тестовые данные для всех таблиц:
- `users`
- `courses`
- `lessons`
- `course_enrollments`
- `lesson_progress`

В каждой таблице добавлено не менее 10 записей, как требуется в задании.


## 3. Создание индексов

### Анализ типовых запросов

В системе выполняются следующие основные операции:

- поиск пользователя по логину;
- поиск пользователя по имени и фамилии;
- получение списка уроков курса;
- получение курсов пользователя;
- получение прогресса пользователя;
- соединение таблиц при выполнении JOIN-запросов.

### Созданные индексы

#### Индексы на первичные ключи

создаются автоматически

#### Индексы

```sql
CREATE INDEX idx_courses_teacher_id
    ON courses(teacher_id);

CREATE INDEX idx_course_enrollments_course_id
    ON course_enrollments(course_id);

CREATE INDEX idx_lesson_progress_lesson_id
    ON lesson_progress(lesson_id);

CREATE INDEX idx_users_last_name_first_name
    ON users(last_name, first_name);
```

**Назначение:** 
- idx_courses_teacher_id — ускоряет поиск курсов по автору;
- idx_course_enrollments_course_id — ускоряет получение пользователей, записанных на курс;
- idx_lesson_progress_lesson_id — ускоряет получение прогресса по конкретному уроку;
- idx_users_last_name_first_name — ускоряет поиск пользователей по фамилии и имени.

Часть индексов создаётся автоматически ограничениями  `UNIQUE`.
Например, ограничения:

- `UNIQUE (login)`
- `UNIQUE (course_id, position)`
- `UNIQUE (user_id, course_id)`
- `UNIQUE (user_id, lesson_id)`

формируют соответствующие уникальные индексы в PostgreSQL.

Дополнительные индексы были оставлены только для тех полей, которые не покрываются составными индексами по префиксу и используются в фильтрации и JOIN-запросах.

Поле `login` имеет ограничение UNIQUE, которое автоматически
создаёт индекс в PostgreSQL. Это позволяет обеспечить уникальность
логинов и ускоряет поиск пользователя по логину.

Для таблицы `lessons` уже используется составной уникальный индекс:

- UNIQUE (course_id, position)

Данный индекс автоматически создаётся PostgreSQL и используется
для ускорения выборки уроков курса с сортировкой по позиции.

Для таблиц `course_enrollments` и `lesson_progress` используются составные
уникальные ограничения:

- UNIQUE (user_id, course_id)
- UNIQUE (user_id, lesson_id)

PostgreSQL автоматически создаёт уникальные индексы для этих ограничений.

Однако дополнительные индексы по отдельным полям (`course_id`, `lesson_id`)
были добавлены, так как составные индексы неэффективны для поиска по
второму полю без использования первого.

## 4. Оптимизация запросов

В соответствии с заданием были подготовлены SQL-запросы для всех операций варианта.
Они размещены в файле [queries.sql](lms_service/sql/queries.sql).

Анализ плана выполнения запросов, их оптимизация и сравнение планов до/после оптимазации находится в файле [optimization.md](optimization.md).

## 5. Партицирование (опционально)

Партиционирование таблиц в рамках данного проекта не применялось.

Это связано с тем, что:
- объём данных в системе является небольшим;
- запросы выполняются быстро за счёт использования индексов;
- отсутствуют сценарии, требующие обработки очень больших таблиц (миллионы записей);
- нет необходимости в разделении данных по диапазонам (например, по дате или категориям).

Партиционирование имеет смысл применять в системах с большим объёмом данных
и высокой нагрузкой, где требуется ускорение выборок за счёт работы только
с частью таблицы. В рамках данного учебного проекта его использование неактуально.

## 6. Подключение API к базе данных

В рамках данного этапа сервис был полностью переведён с in-memory хранилища на использование PostgreSQL.

### Выполненные изменения

- Подключён компонент `userver::components::Postgres` и настроено подключение к базе данных.
- Реализован класс `PostgresStorage`, инкапсулирующий работу с БД.
- Все обработчики (handlers) переведены на использование PostgreSQL.

### Особенности реализации

- Используются параметризованные SQL-запросы (`$1`, `$2`), что предотвращает SQL-инъекции.
- Для ускорения работы применяются индексы (в том числе автоматически создаваемые через `UNIQUE`).
- Для предотвращения дублирования записей используются конструкции `ON CONFLICT`.
- Взаимодействие с БД осуществляется через `ClusterPtr` из userver.

### Проверка работоспособности

Корректность работы была проверена с помощью:
- HTTP-запросов (`curl`) ко всем endpoint’ам;
- прямых SQL-запросов к базе данных через `psql`.

Изменения в базе данных напрямую отражаются в ответах API, что подтверждает использование PostgreSQL в качестве основного источника данных.

## Запуск проекта

### 1. Перейти в каталог сервиса

```bash
cd lms_service
```
### 2. Поднять PostgreSQL
```bash
docker compose up -d postgres
```
### 3. Инициализировать базу данных
```bash
docker exec -i lms_postgres psql -U postgres -d lms_db < sql/schema.sql
docker exec -i lms_postgres psql -U postgres -d lms_db < sql/data.sql
```

#### Проверка корректности заполнения

Подключиться к БД внутри контейнера
```bash
docker exec -it lms_postgres psql -U postgres -d lms_db
```
Ты попадёшь в `lms_db=#`

Для проверки количества записей в таблицах выполнить запрос:

```sql
SELECT 'users' AS table_name, COUNT(*) FROM users
UNION ALL
SELECT 'courses', COUNT(*) FROM courses
UNION ALL
SELECT 'lessons', COUNT(*) FROM lessons
UNION ALL
SELECT 'course_enrollments', COUNT(*) FROM course_enrollments
UNION ALL
SELECT 'lesson_progress', COUNT(*) FROM lesson_progress;
```

Результат должен показать, что в каждой таблице содержится по 10 записей,
что соответствует требованиям задания.

### 4. Запустить API в Docker
```bash
docker compose up --build lms-service
```
После запуска API будет доступно по адресу: `http://localhost:8080`

### 5. Запуск тестов
```bash
make test-debug
```
