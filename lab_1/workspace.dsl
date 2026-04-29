workspace "LMS" "Система управления обучением"  {
  !identifiers hierarchical

  model {

    guest = person "Гость" "Неавторизованный пользователь системы"
    student = person "Студент" "Пользователь, проходящий обучение на платформе"
    teacher = person "Преподаватель" "Пользователь, создающий образовательный контент"
    admin = person "Администратор" "Пользователь, управляющий системой и учетными записями"

    emailSystem = softwareSystem "Email-сервис" "Система отправки писем подтверждения регистрации и уведомлений" {
      tags "External"
    }

    lms = softwareSystem "LMS" "Система управления обучением" {

      spa = container "Web Application" "Пользовательский интерфейс для студентов и преподавателей" "Angular (SPA)" {
        tags "WebApp"
      }

      apiGateway = container "API Gateway" "Единая точка входа для маршрутизации запросов, проверки JWT и ограничения частоты запросов" "NGINX / Kong" {
        tags "Gateway"
      }

      userService = container "User Service" "Сервис регистрации, авторизации, поиска пользователей и управления учетными записями. Реализует кеширование пользователей и rate limiting для авторизации" "C++ / userver (REST)"

      courseService = container "Course Service" "Сервис создания курсов, получения списка курсов, добавления уроков и получения уроков курса. Данные курсов и уроков хранятся в MongoDB" "C++ / userver (REST)"

      enrollmentService = container "Enrollment Service" "Сервис записи пользователей на курсы, получения курсов пользователя и отметки о прохождении уроков" "C++ / userver (REST)"

      postgres = container "PostgreSQL" "Реляционная база данных пользователей, записей на курсы и прогресса обучения" "PostgreSQL" {
        tags "Database"
      }

      mongo = container "MongoDB" "Документо-ориентированная база данных курсов и уроков" "MongoDB" {
        tags "Database"
      }

      redis = container "Redis" "In-memory key-value хранилище для кеширования часто читаемых данных и хранения счетчиков rate limiting" "Redis" {
        tags "Cache"
      }

    }

    guest -> lms "Просматривает курсы" "HTTPS"
    student -> lms "Проходит обучение" "HTTPS"
    teacher -> lms "Создает курсы и уроки" "HTTPS"
    admin -> lms "Администрирует систему" "HTTPS"

    lms -> emailSystem "Отправляет email уведомления" "SMTP"

    guest -> lms.spa "Просматривает курсы" "HTTPS"
    student -> lms.spa "Проходит обучение" "HTTPS"
    teacher -> lms.spa "Создает курсы и уроки" "HTTPS"
    admin -> lms.spa "Управляет системой" "HTTPS"

    lms.spa -> lms.apiGateway "Выполняет API-запросы" "HTTPS/REST"

    lms.apiGateway -> lms.userService "Маршрутизирует /auth и /users" "HTTPS/REST"
    lms.apiGateway -> lms.courseService "Маршрутизирует /courses и /lessons" "HTTPS/REST"
    lms.apiGateway -> lms.enrollmentService "Маршрутизирует /enrollments и /progress" "HTTPS/REST"

    lms.userService -> lms.postgres "Читает и записывает пользователей" "SQL"
    lms.userService -> lms.redis "Читает/записывает кеш пользователей, результаты поиска и счетчики rate limiting" "Redis protocol"
    lms.userService -> emailSystem "Отправляет подтверждение регистрации" "SMTP"

    lms.courseService -> lms.mongo "Читает и записывает курсы и уроки" "NoSQL"

    lms.enrollmentService -> lms.postgres "Читает и записывает записи на курсы и прогресс обучения" "SQL"
    lms.enrollmentService -> emailSystem "Отправляет уведомление о записи на курс/прохождение урока" "SMTP"

  }

  views {

    systemContext lms "SystemContext" "Диаграмма контекста системы (С1)"  {
      include *
      autolayout lr
    }

    container lms "Containers" "Диаграмма контейнеров системы (C2)" {
      include *
      autolayout lr
    }

    dynamic lms "Get_User_By_Login_With_Cache" "Получение пользователя по логину с использованием кеша" {
      autolayout lr

      student -> lms.spa "Открывает профиль пользователя"
      lms.spa -> lms.apiGateway "GET /users/by-login/{login} (JWT)"
      lms.apiGateway -> lms.userService "Перенаправляет запрос"
      lms.userService -> lms.redis "GET user:login:{login}"

      lms.userService -> lms.postgres "SELECT user by login при cache miss"
      lms.userService -> lms.redis "SET user:login:{login} TTL 300 sec"
    }

    dynamic lms "Login_With_Rate_Limiting" "Авторизация пользователя с rate limiting" {
      autolayout lr

      student -> lms.spa "Вводит логин и пароль"
      lms.spa -> lms.apiGateway "POST /auth/login"
      lms.apiGateway -> lms.userService "Перенаправляет запрос авторизации"
      lms.userService -> lms.redis "Проверяет счетчик rate_limit:login:{client}:{window}"
      lms.userService -> lms.postgres "Проверяет логин и пароль"
    }

    dynamic lms "Add_Student_At_Course" "Запись пользователя на курс" {
      autolayout lr

      student -> lms.spa "Открывает страницу курса"
      lms.spa -> lms.apiGateway "POST /enrollments (JWT)"
      lms.apiGateway -> lms.enrollmentService "Перенаправляет запрос"
      lms.enrollmentService -> lms.postgres "INSERT enrollment"
    }

    styles {

      element "Person" {
        shape person
        background #08427b
        color #ffffff
      }

      element "Software System" {
        background #1168bd
        color #ffffff
      }

      element "External" {
        background #999999
        color #ffffff
      }

      element "Container" {
        background #438dd5
        color #ffffff
      }

      element "WebApp" {
        shape webbrowser
      }

      element "Gateway" {
        shape hexagon
      }

      element "Database" {
        shape cylinder
      }

      element "Cache" {
        shape cylinder
        background #ff8c00
        color #ffffff
      }

    }

  }

}