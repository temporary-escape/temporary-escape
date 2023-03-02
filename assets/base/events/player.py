from engine import create_logger, event_bus, EventPlayerLoggedIn  # noqa

logger = create_logger(__name__)


@event_bus.listen('other')
def event_other(event: dict):
    logger.info(f'event_other() called with: {event}')


@event_bus.listen('player_logged_in')
def event_player_logged_in(event: EventPlayerLoggedIn):
    logger.info(f'event_player_logged_in() called with player: {event.player_id}')
