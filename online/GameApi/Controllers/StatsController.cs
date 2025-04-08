using GameApi.Services;
using Microsoft.AspNetCore.Authorization;
using Microsoft.AspNetCore.Mvc;

namespace GameApi.Controllers
{
    [ApiController]
    [Authorize]
    [Route("[controller]")]
    public class StatsController : ControllerBase
    {
        private readonly StatsService _statsService;

        public StatsController(StatsService statsService)
        {
            _statsService = statsService;
        }

        [HttpGet]
        public async Task<IActionResult> Get()
        {
            var username = User.Identity?.Name!;
            var stats = await _statsService.GetStatsAsync(username);
            return stats == null ? NotFound() : Ok(stats);
        }

        [HttpPost("{username}")]
        public async Task<IActionResult> Set(string username, [FromBody] PlayerStats stats)
        {
            await _statsService.SetStatsAsync(username, stats);
            return Ok();
        }
    }
}