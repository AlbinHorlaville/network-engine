using Microsoft.AspNetCore.Authentication.JwtBearer;
using Microsoft.IdentityModel.Tokens;
using Microsoft.OpenApi.Models;
using System.Text;
using StackExchange.Redis;
using GameApi.Services;

var redisPort = 6379;

var builder = WebApplication.CreateBuilder(args);

// Ajout de la lecture du fichier appsettings.json
builder.Configuration.AddJsonFile("appsettings.json", optional: false, reloadOnChange: true);

// Ajout de la configuration de l'authentification JWT
var key = builder.Configuration["JwtKey"]; // Récupérer la clé depuis appsettings.json

builder.Services.AddAuthentication(JwtBearerDefaults.AuthenticationScheme)
    .AddJwtBearer(options =>
    {
        options.TokenValidationParameters = new TokenValidationParameters
        {
            ValidateIssuer = false,
            ValidateAudience = false,
            ValidateLifetime = true,
            ValidateIssuerSigningKey = true,
            IssuerSigningKey = new SymmetricSecurityKey(Encoding.ASCII.GetBytes(key)),
            RequireSignedTokens = true,
            SignatureValidator = null, // facultatif
            ValidAlgorithms = new[] { SecurityAlgorithms.HmacSha512 } // important
        };
    });

builder.Services.AddAuthorization();


// Add services to the container.

builder.Services.AddControllers();

builder.Services.AddEndpointsApiExplorer();

// Register Redis as a singleton service.
builder.Services.AddSingleton<IConnectionMultiplexer>(ConnectionMultiplexer.Connect("localhost:" + redisPort));

builder.Services.AddScoped<AuthService>();
builder.Services.AddScoped<StatsService>();
builder.Services.AddScoped<AchievementService>();
builder.Services.AddScoped<MatchmakingService>();
builder.Services.AddHostedService<MatchMakingWorker>();

builder.Services.AddSwaggerGen(c =>
{
    c.SwaggerDoc("v1", new() { Title = "My API", Version = "v1" });

    c.AddSecurityDefinition("Bearer", new()
    {
        Name = "Authorization",
        Type = SecuritySchemeType.ApiKey,
        Scheme = "Bearer",
        BearerFormat = "JWT",
        In = ParameterLocation.Header,
        Description = "Enter 'Bearer' [space] and then your token"
    });

    c.AddSecurityRequirement(new OpenApiSecurityRequirement
    {
        {
            new OpenApiSecurityScheme {
                Reference = new OpenApiReference {
                    Type = ReferenceType.SecurityScheme,
                    Id = "Bearer"
                }
            },
            new string[] {}
        }
    });
});

builder.WebHost.ConfigureKestrel(serverOptions =>
{
    serverOptions.ListenAnyIP(5187);
});

var app = builder.Build();

// Create server user in Redis if it doesn't exist
using (var scope = app.Services.CreateScope())
{
    var redis = scope.ServiceProvider.GetRequiredService<IConnectionMultiplexer>();
    var db = redis.GetDatabase();

    const string serverUsername = "Server";
    const string serverPassword = "SecuredPassword"; // Change this to something strong and secure

    if (!await db.HashExistsAsync("users", serverUsername))
    {
        await db.HashSetAsync("users", serverUsername, serverPassword);
        Console.WriteLine("Server user created in Redis.");
    }
}

// Configure the HTTP request pipeline.
if (app.Environment.IsDevelopment())
{
    app.UseSwagger();
    app.UseSwaggerUI();
}

app.UseHttpsRedirection();

app.UseAuthorization();

app.MapControllers();

app.Run();
