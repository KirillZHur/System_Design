# 1. Анализ событий в системе

## 1.1 Основные сущности

Система LMS содержит следующие основные сущности:
- User;
- Course;
- Lesson;
- Enrollment;
- Progress.

Для реализации CQRS выбрана сущность `Course`.

---

## 1.2 Команды системы

Команда представляет собой запрос на изменение состояния системы.

| Команда | Описание |
|---|---|
| `CreateUser` | Создание пользователя |
| `CreateCourse` | Создание курса |
| `AddLessonToCourse` | Добавление урока в курс |
| `EnrollUserToCourse` | Запись пользователя на курс |
| `MarkLessonCompleted` | Отметка урока как завершённого |

---

## 1.3 События системы

Событие представляет собой факт уже произошедшего изменения.

| Событие | Описание |
|---|---|
| `UserCreated` | Пользователь создан |
| `CourseCreateRequested` | Получена команда на создание курса |
| `CourseCreated` | Курс успешно сохранён |
| `LessonAddedToCourse` | В курс добавлен урок |
| `UserEnrolledToCourse` | Пользователь записан на курс |
| `LessonCompleted` | Урок завершён пользователем |

## 1.4 Подписчики событий

### UserCreated

Подписчики:
- Auth Service;
- User Service.

### CourseCreateRequested

Подписчики:
- Course Consumer Service.

### CourseCreated

Подписчики:
- Course Service;
- Enrollment Service;
- Lesson Service.

### LessonAddedToCourse

Подписчики:
- Course Service;
- Progress Service.

### UserEnrolledToCourse

Подписчики:
- Progress Service;
- Course Service.

### LessonCompleted

Подписчики:
- Progress Service;
- Course Service.

# 2. Проектирование Event-Driven архитектуры

## 2.1 Event producers

В системе выделены следующие производители событий.

| Producer | Тип сообщения | Topic | Назначение |
|---|---|---|---|
| Course Command Service | `CourseCreateRequested` | `lms.course.commands` | Публикует команду на создание курса |
| Course Consumer Service | `CourseCreated` | `lms.course.events` | Публикует факт успешного создания курса |
| User Service | `UserCreated` | `lms.user.events` | Публикует факт создания пользователя |
| Lesson Service | `LessonAddedToCourse` | `lms.lesson.events` | Публикует факт добавления урока |
| Enrollment Service | `UserEnrolledToCourse` | `lms.enrollment.events` | Публикует факт записи пользователя |
| Progress Service | `LessonCompleted` | `lms.progress.events` | Публикует факт завершения урока |

В рамках программной реализации ЛР 6 реализуется producer для `CourseCreateRequested`.

## 2.2 Event consumers

В системе выделены следующие потребители событий.

| Consumer | Topic | Обрабатываемое сообщение | Действие |
|---|---|---|---|
| Course Consumer Service | `lms.course.commands` | `CourseCreateRequested` | Сохраняет курс в PostgreSQL и MongoDB |
| Course Service | `lms.course.events` | `CourseCreated` | Использует read-модель курсов |
| Lesson Service | `lms.course.events` | `CourseCreated` | Проверяет существование курса при работе с уроками |
| Enrollment Service | `lms.course.events` | `CourseCreated` | Проверяет доступность курса для записи |
| Progress Service | `lms.lesson.events`, `lms.enrollment.events` | `LessonAddedToCourse`, `UserEnrolledToCourse` | Обновляет прогресс пользователя |

В рамках программной реализации ЛР 6 реализуется consumer для `CourseCreateRequested`.


## 2.3 Типы событий и структура payload

Для обмена сообщениями используется JSON.

Базовая структура сообщения:

```json
{
  "event_id": "uuid",
  "event_type": "string",
  "occurred_at": "datetime",
  "payload": {}
}
```
`CourseCreateRequested`
```json
{
  "event_id": "uuid",
  "event_type": "CourseCreateRequested",
  "occurred_at": "2026-05-12T12:00:00Z",
  "payload": {
    "title": "System Design",
    "description": "Course about software architecture",
    "teacher_id": 1
  }
}
```
`CourseCreated`
```json
{
  "event_id": "uuid",
  "event_type": "CourseCreated",
  "occurred_at": "2026-05-12T12:00:05Z",
  "payload": {
    "course_id": 1,
    "title": "System Design",
    "description": "Course about software architecture",
    "teacher_id": 1
  }
}
```
## 2.4 Поток событий

Поток обработки команды создания курса:

1. Клиент отправляет запрос `POST /api/v1/courses.`
2. `Course Command Service` создаёт сообщение `CourseCreateRequested`.
3. Сообщение публикуется в Kafka topic `lms.course.commands`.
4. `Course Consumer Service` получает сообщение из Kafka.
5. Consumer сохраняет курс в PostgreSQL.
6. Consumer сохраняет документ курса в MongoDB.
7. После успешной записи курс становится доступен через read-модель.
8. При необходимости consumer может опубликовать событие CourseCreated в topic `lms.course.events`.

# 3. Проектирование взаимодействия через брокер сообщений

## 3.1 Выбор брокера сообщений

Для реализации событийного взаимодействия выбран **Apache Kafka**.

Kafka используется как брокер сообщений между командной частью системы и обработчиком команд.

Основной сценарий:

```text
Course Command Service -> Kafka -> Course Consumer Service
```

Kafka выбрана по следующим причинам:

| Возможность Kafka   | Значение для LMS                                             |
| ------------------- | ------------------------------------------------------------ |
| Topic-based модель  | Команды и события можно разделить по отдельным topic         |
| Хранение сообщений  | Сообщения сохраняются и могут быть перечитаны                |
| Offset              | Consumer контролирует, какие сообщения уже обработаны        |
| Consumer group      | Можно масштабировать обработчики команд                      |
| Порядок в partition | Команды по одной сущности можно обрабатывать последовательно |
| Поддержка CQRS      | Kafka подходит для синхронизации write и read моделей        |

## 3.2 Topics

В рамках проектирования используются следующие Kafka topics.

| Topic                 | Тип сообщений | Producer                | Consumer                                           | Назначение                         |
| --------------------- | ------------- | ----------------------- | -------------------------------------------------- | ---------------------------------- |
| `lms.course.commands` | Command event | Course Command Service  | Course Consumer Service                            | Передача команды на создание курса |
| `lms.course.events`   | Domain event  | Course Consumer Service | Course Service, Lesson Service, Enrollment Service | Публикация факта создания курса    |

В программной реализации ЛР 6 обязательным является topic `lms.course.commands`. Topic `lms.course.events` описан как архитектурное расширение для публикации события `CourseCreated`.

## 3.3 Формат сообщений

Для сообщений выбран формат JSON.

Базовый контракт сообщения:
```json
{
  "event_id": "uuid",
  "event_type": "string",
  "occurred_at": "datetime",
  "payload": {}
}
```

Назначение полей:

| Поле          | Тип    | Назначение                         |
| ------------- | ------ | ---------------------------------- |
| `event_id`    | string | Уникальный идентификатор сообщения |
| `event_type`  | string | Тип команды или события            |
| `occurred_at` | string | Дата и время создания сообщения    |
| `payload`     | object | Данные команды или события         |

## 3.4 Сообщение CourseCreateRequested
```json
{
  "event_id": "a7f0c5e2-5a7a-4df4-b1d1-0d4f4b4d2b1a",
  "event_type": "CourseCreateRequested",
  "occurred_at": "2026-05-12T12:00:00Z",
  "payload": {
    "title": "System Design",
    "description": "Course about software architecture",
    "teacher_id": 1
  }
}
```
Это сообщение публикуется в topic `lms.course.commands`.

## 3.5 Сообщение CourseCreated
```json
{
  "event_id": "b9e6b97c-1892-4c85-9d10-f4d5d989401a",
  "event_type": "CourseCreated",
  "occurred_at": "2026-05-12T12:00:05Z",
  "payload": {
    "course_id": 1,
    "title": "System Design",
    "description": "Course about software architecture",
    "teacher_id": 1
  }
}
```
Это сообщение может публиковаться в topic `lms.course.events` после успешного сохранения курса.

## 3.6 Гарантии доставки

Для взаимодействия используется гарантия доставки `at-least-once`.

При такой гарантии Kafka обеспечивает доставку сообщения минимум один раз, но допускается повторная доставка при сбоях.

Последовательность обработки:

1. Course Consumer Service получает сообщение из Kafka.
2. Consumer сохраняет курс в PostgreSQL.
3. Consumer сохраняет агрегат курса в MongoDB.
4. Только после успешного сохранения данных consumer фиксирует offset.
5. Если во время обработки произошёл сбой, сообщение будет обработано повторно.

## 3.7 Идемпотентность обработки

Так как используется `at-least-once`, consumer должен корректно обрабатывать повторную доставку одного и того же сообщения.

Для этого используется:

- уникальный event_id;
- таблица обработанных сообщений;
- проверка существования обработанного события;
- фиксация offset только после успешной транзакции.

Логика обработки:
```texr
если event_id уже обработан:
    пропустить сообщение
иначе:
    сохранить курс
    сохранить event_id как обработанный
    зафиксировать offset
```    

### Почему не exactly-once

Гарантия `exactly-once` в данной реализации не используется.

Причины:

- требуется настройка транзакционного producer/consumer в Kafka;
- необходимо согласовывать транзакции Kafka с PostgreSQL и MongoDB;
- для ЛР достаточно at-least-once с идемпотентным consumer.


# 4. Применение паттерна CQRS

## 4.1 Обоснование применения CQRS

CQRS применяется в LMS для разделения операций изменения состояния и операций чтения.

Для реализации выбран bounded context `Course`, так как создание курса является write-операцией, а получение списка курсов и уроков относится к read-операциям.

В рамках ЛР 6 CQRS реализуется для команды создания курса.

## 4.2 Command model

Command model отвечает за обработку команд.

Команда:

| Команда | Описание | Обработчик |
|---|---|---|
| `CreateCourse` | Создание нового курса | Course Command Service |

Командный сервис не сохраняет курс напрямую в базу данных. Вместо этого он публикует сообщение в Kafka.

```text
POST /api/v1/courses
  -> Course Command Service
  -> CourseCreateRequested
  -> Kafka topic lms.course.commands
```

## 4.3 Query model

Query model отвечает за чтение данных.

| Query              | Описание                          | Источник данных      |
| ------------------ | --------------------------------- | -------------------- |
| `GetCourses`       | Получение списка курсов           | PostgreSQL / MongoDB |
| `GetCourseById`    | Получение курса по идентификатору | PostgreSQL / MongoDB |
| `GetCourseLessons` | Получение уроков курса            | MongoDB              |
| `GetUserCourses`   | Получение курсов пользователя     | PostgreSQL           |

Query-операции не публикуют события и не изменяют состояние системы.

## 4.4 Синхронизация моделей

Синхронизация command model и query model выполняется через Kafka.

Порядок работы:

1. `Course Command Service` принимает команду `CreateCourse`.
2. Сервис создаёт сообщение `CourseCreateRequested`.
3. Сообщение публикуется в Kafka topic `lms.course.commands`.
4. `Course Consumer Service` читает сообщение.
5. Consumer сохраняет курс в PostgreSQL.
6. Consumer сохраняет агрегат курса в MongoDB.
7. Данные становятся доступны для query-операций.

## 4.5 Eventual consistency

После отправки команды данные появляются в read model не сразу.

Это означает, что система работает в модели eventual consistency.

Клиент получает ответ о принятии команды, но фактическое появление курса в списке курсов происходит после обработки сообщения consumer-сервисом.

## 4.6 Преимущества CQRS для LMS

| Преимущество               | Проявление в LMS                                               |
| -------------------------- | -------------------------------------------------------------- |
| Разделение ответственности | Command Service отвечает за запись, Course Service — за чтение |
| Слабая связанность         | Command Service не зависит напрямую от базы данных             |
| Масштабируемость           | Consumer можно масштабировать отдельно                         |
| Устойчивость к нагрузке    | Kafka сглаживает поток команд                                  |
| Расширяемость              | Можно добавить новых подписчиков на события                    |

## 4.7 Ограничения

CQRS усложняет систему, так как появляется асинхронная обработка и задержка между записью и чтением.

Для корректной работы необходимо учитывать:

- повторную доставку сообщений;
- идемпотентность consumer-сервиса;
- возможное временное отставание read model;
- обработку ошибок при записи в базы данных.

# 5. Реализация Event-Driven сценария

## 5.1 Реализуемый сценарий

В рамках программной части ЛР 6 реализуется сценарий создания курса через Kafka.

Выбранная сущность: `Course`.

Команда: `CreateCourse`.

Сообщение в Kafka: `CourseCreateRequested`.

Основной поток:

```text
Client
  -> Course Command Service
  -> Kafka topic lms.course.commands
  -> Course Consumer Service
  -> PostgreSQL / MongoDB
```

## 5.2 Course Command Service

`Course Command Service` является producer-сервисом.

Он реализуется как отдельный C++ userver-сервис.

Ответственность сервиса:

- принять HTTP-запрос на создание курса;
- проверить корректность входных данных;
- сформировать JSON-сообщение `CourseCreateRequested`;
- сохранить команду в Redis;
- отправить сообщение в Kafka topic `lms.course.commands`;
вернуть клиенту `202 Accepted`.

Сервис не выполняет прямую запись курса в PostgreSQL или MongoDB.

## 5.3 Course Consumer Service

`Course Consumer Service` является consumer-сервисом.

Он реализуется как отдельный C++ userver-сервис и запускается в отдельном Docker-контейнере.

Ответственность сервиса:

- подписаться на topic `lms.course.commands`;
- читать сообщения `CourseCreateRequested`;
- выполнять идемпотентную обработку сообщения;
- сохранять курс в PostgreSQL;
- сохранять агрегат курса в MongoDB;
- фиксировать offset после успешной обработки.