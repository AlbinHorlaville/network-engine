using Microsoft.AspNetCore.Mvc;
using Microsoft.IdentityModel.Tokens;
using System.IdentityModel.Tokens.Jwt;
using System.Security.Claims;
using System.Text;

namespace GameApi.Controllers
{

    [ApiController]
    [Route("[controller]")]
    public class AuthController : ControllerBase
    {
        private readonly string jwtKey = "this_is_a_very_super_secret_key_123456!";

        private readonly Dictionary<string, string> users = new()
        {
            { "alice", "password123" },
            { "bob", "hunter2" }
        };

        [HttpPost("login")]
        public IActionResult Login([FromBody] LoginRequest req)
        {
            if (!users.ContainsKey(req.Username) || users[req.Username] != req.Password)
                return Unauthorized();

            var tokenHandler = new JwtSecurityTokenHandler();
            var tokenKey = Encoding.ASCII.GetBytes(jwtKey);
            var token = new JwtSecurityToken(
                claims: new[] { new Claim(ClaimTypes.Name, req.Username) },
                expires: DateTime.UtcNow.AddHours(1),
                signingCredentials: new SigningCredentials(new SymmetricSecurityKey(tokenKey), SecurityAlgorithms.HmacSha256Signature)
            );

            return Ok(new { Token = tokenHandler.WriteToken(token) });
        }
    }

    public class LoginRequest
    {
        public string Username { get; set; }
        public string Password { get; set; }
    }
}