db = db.getSiblingDB('lms_mongo_db');

db.courses.insertMany([
  {
    _id: NumberInt(1),
    title: "Основы программирования",
    description: "Введение в программирование",
    teacher_id: NumberInt(1),
    created_at: ISODate("2026-04-01T10:00:00Z"),
    lessons: [
      {
        id: NumberInt(1),
        title: "Что такое программа",
        content: "Введение в понятие программы",
        position: NumberInt(1),
        created_at: ISODate("2026-04-01T10:10:00Z")
      },
      {
        id: NumberInt(2),
        title: "Переменные и типы данных",
        content: "Изучение базовых типов данных",
        position: NumberInt(2),
        created_at: ISODate("2026-04-01T10:20:00Z")
      }
    ]
  },
  {
    _id: NumberInt(2),
    title: "Алгоритмы и структуры данных",
    description: "Базовые алгоритмы и структуры данных",
    teacher_id: NumberInt(2),
    created_at: ISODate("2026-04-02T10:00:00Z"),
    lessons: [
      {
        id: NumberInt(1),
        title: "Массивы",
        content: "Работа с массивами",
        position: NumberInt(1),
        created_at: ISODate("2026-04-02T10:10:00Z")
      },
      {
        id: NumberInt(2),
        title: "Связные списки",
        content: "Структура связного списка",
        position: NumberInt(2),
        created_at: ISODate("2026-04-02T10:20:00Z")
      }
    ]
  },
  {
    _id: NumberInt(3),
    title: "Базы данных",
    description: "Основы проектирования баз данных",
    teacher_id: NumberInt(3),
    created_at: ISODate("2026-04-03T10:00:00Z"),
    lessons: [
      {
        id: NumberInt(1),
        title: "Реляционные базы данных",
        content: "Принципы работы реляционных СУБД",
        position: NumberInt(1),
        created_at: ISODate("2026-04-03T10:10:00Z")
      },
      {
        id: NumberInt(2),
        title: "Нормализация",
        content: "Нормальные формы",
        position: NumberInt(2),
        created_at: ISODate("2026-04-03T10:20:00Z")
      }
    ]
  },
  {
    _id: NumberInt(4),
    title: "Компьютерные сети",
    description: "Основы сетевых технологий",
    teacher_id: NumberInt(4),
    created_at: ISODate("2026-04-04T10:00:00Z"),
    lessons: [
      {
        id: NumberInt(1),
        title: "Модель OSI",
        content: "Уровни модели OSI",
        position: NumberInt(1),
        created_at: ISODate("2026-04-04T10:10:00Z")
      },
      {
        id: NumberInt(2),
        title: "TCP/IP",
        content: "Основы протоколов TCP/IP",
        position: NumberInt(2),
        created_at: ISODate("2026-04-04T10:20:00Z")
      }
    ]
  },
  {
    _id: NumberInt(5),
    title: "Операционные системы",
    description: "Принципы работы операционных систем",
    teacher_id: NumberInt(5),
    created_at: ISODate("2026-04-05T10:00:00Z"),
    lessons: [
      {
        id: NumberInt(1),
        title: "Процессы и потоки",
        content: "Организация выполнения программ",
        position: NumberInt(1),
        created_at: ISODate("2026-04-05T10:10:00Z")
      },
      {
        id: NumberInt(2),
        title: "Память",
        content: "Управление памятью",
        position: NumberInt(2),
        created_at: ISODate("2026-04-05T10:20:00Z")
      }
    ]
  },
  {
    _id: NumberInt(6),
    title: "Объектно-ориентированное программирование",
    description: "Основы ООП",
    teacher_id: NumberInt(1),
    created_at: ISODate("2026-04-06T10:00:00Z"),
    lessons: [
      {
        id: NumberInt(1),
        title: "Классы и объекты",
        content: "Основные понятия ООП",
        position: NumberInt(1),
        created_at: ISODate("2026-04-06T10:10:00Z")
      },
      {
        id: NumberInt(2),
        title: "Наследование",
        content: "Механизм наследования",
        position: NumberInt(2),
        created_at: ISODate("2026-04-06T10:20:00Z")
      }
    ]
  },
  {
    _id: NumberInt(7),
    title: "Web-разработка",
    description: "Создание веб-приложений",
    teacher_id: NumberInt(2),
    created_at: ISODate("2026-04-07T10:00:00Z"),
    lessons: [
      {
        id: NumberInt(1),
        title: "HTTP",
        content: "Протокол HTTP",
        position: NumberInt(1),
        created_at: ISODate("2026-04-07T10:10:00Z")
      },
      {
        id: NumberInt(2),
        title: "REST API",
        content: "Принципы REST",
        position: NumberInt(2),
        created_at: ISODate("2026-04-07T10:20:00Z")
      }
    ]
  },
  {
    _id: NumberInt(8),
    title: "Тестирование программного обеспечения",
    description: "Методы тестирования ПО",
    teacher_id: NumberInt(3),
    created_at: ISODate("2026-04-08T10:00:00Z"),
    lessons: [
      {
        id: NumberInt(1),
        title: "Unit-тесты",
        content: "Модульное тестирование",
        position: NumberInt(1),
        created_at: ISODate("2026-04-08T10:10:00Z")
      },
      {
        id: NumberInt(2),
        title: "Интеграционные тесты",
        content: "Интеграционное тестирование",
        position: NumberInt(2),
        created_at: ISODate("2026-04-08T10:20:00Z")
      }
    ]
  },
  {
    _id: NumberInt(9),
    title: "Информационная безопасность",
    description: "Основы защиты информации",
    teacher_id: NumberInt(4),
    created_at: ISODate("2026-04-09T10:00:00Z"),
    lessons: [
      {
        id: NumberInt(1),
        title: "Аутентификация",
        content: "Методы проверки подлинности",
        position: NumberInt(1),
        created_at: ISODate("2026-04-09T10:10:00Z")
      },
      {
        id: NumberInt(2),
        title: "Авторизация",
        content: "Разграничение прав доступа",
        position: NumberInt(2),
        created_at: ISODate("2026-04-09T10:20:00Z")
      }
    ]
  },
  {
    _id: NumberInt(10),
    title: "Архитектура программных систем",
    description: "Проектирование архитектуры приложений",
    teacher_id: NumberInt(5),
    created_at: ISODate("2026-04-10T10:00:00Z"),
    lessons: [
      {
        id: NumberInt(1),
        title: "Монолит и микросервисы",
        content: "Подходы к архитектуре",
        position: NumberInt(1),
        created_at: ISODate("2026-04-10T10:10:00Z")
      },
      {
        id: NumberInt(2),
        title: "Выбор хранилища",
        content: "SQL и NoSQL базы данных",
        position: NumberInt(2),
        created_at: ISODate("2026-04-10T10:20:00Z")
      }
    ]
  }
]);