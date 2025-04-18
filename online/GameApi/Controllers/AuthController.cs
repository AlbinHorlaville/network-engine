using GameApi.Services;
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
        private readonly AuthService _authService;
        private readonly IConfiguration _config;

        public AuthController(AuthService authService, IConfiguration config)
        {
            _authService = authService;
            _config = config;
        }

        [HttpPost("register")]
        public async Task<IActionResult> Register([FromBody] LoginRequest req)
        {
            if (req.Password.Length < 8 || !System.Text.RegularExpressions.Regex.IsMatch(req.Password, @"^[a-zA-Z0-9]+$"))
                return BadRequest("Password must be alphanumerical and at least 8 characters.");

            var success = await _authService.RegisterAsync(req.Username, req.Password);
            if (!success)
                return Unauthorized();

            return Ok(new { Token = GenerateToken(req.Username) });
        }

        [HttpPost("login")]
        public async Task<IActionResult> Login([FromBody] LoginRequest req)
        {
            if (!await _authService.ValidateAsync(req.Username, req.Password))
                return Unauthorized();

            return Ok(new { Token = GenerateToken(req.Username) });
        }

        private string GenerateToken(string username)
        {
            var key = Encoding.ASCII.GetBytes(_config["JwtKey"]);
            var tokenHandler = new JwtSecurityTokenHandler();
            var token = new JwtSecurityToken(
                claims: new[] { new Claim(ClaimTypes.Name, username) },
                expires: DateTime.UtcNow.AddHours(1),
                signingCredentials: new SigningCredentials(new SymmetricSecurityKey(key), SecurityAlgorithms.HmacSha512)
            );
            return tokenHandler.WriteToken(token);
        }
    }

    public class LoginRequest
    {
        public string Username { get; set; } = "";
        public string Password { get; set; } = "";
    }
}