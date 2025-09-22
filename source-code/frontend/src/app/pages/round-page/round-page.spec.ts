import { ComponentFixture, TestBed } from '@angular/core/testing';

import { RoundPage } from './round-page';

describe('RoundPage', () => {
  let component: RoundPage;
  let fixture: ComponentFixture<RoundPage>;

  beforeEach(async () => {
    await TestBed.configureTestingModule({
      imports: [RoundPage]
    })
    .compileComponents();

    fixture = TestBed.createComponent(RoundPage);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
