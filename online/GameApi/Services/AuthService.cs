using StackExchange.Redis;

namespace GameApi.Services
{
    public class AuthService
    {
        private readonly IDatabase _db;

        public AuthService(IConnectionMultiplexer redis)
        {
            _db = redis.GetDatabase();
        }

        public async Task<bool> RegisterAsync(string username, string password)
        {
            var exists = await _db.HashExistsAsync("users", username);
            if (exists)
            {
                return false;
            }

            await _db.HashSetAsync("users", username, password);
            return true;
        }

        public async Task<bool> ValidateAsync(string username, string password)
        {
            var stored = await _db.HashGetAsync("users", username);
            return stored == password;
        }
    }
}
